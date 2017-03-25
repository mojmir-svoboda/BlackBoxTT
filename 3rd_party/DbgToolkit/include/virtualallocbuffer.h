#pragma once
#include <string>
#include <cassert>
#include <windows.h>

struct VirtualAllocBuffer
{
	char * m_region_base_addr;
	size_t m_end_offset;
	size_t m_region_size;
	size_t m_region_capacity;

	VirtualAllocBuffer () : m_region_base_addr(nullptr), m_end_offset(0), m_region_size(0), m_region_capacity(0) { }

	~VirtualAllocBuffer ()
	{
		rmReservedPages();
	}

	char * begin () { return m_region_base_addr; }
	char const * begin () const { return m_region_base_addr; }
	char * end () { return begin() + m_end_offset; }
	char const * end () const { return begin() + m_end_offset; }
	size_t capacity () const { return m_region_size; }
	size_t shiftEnd (size_t n) {  return m_end_offset += n; }
	void reset () { m_end_offset = 0; }

	bool mkReservedPages (size_t reserved_region_size)
	{
		LPVOID addr = VirtualAlloc(nullptr, reserved_region_size, MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
		if (m_region_base_addr = static_cast<char *>(addr))
		{
			qDebug("MEMORY: virtualalloc at base=0x%x sz=%u", m_region_base_addr, reserved_region_size);
			m_region_capacity = reserved_region_size;
			return true;
		}
		assert(m_region_base_addr);
		return false;
	}
	bool mkCommitPages (char * addr, size_t commited_region_size)
	{
		LPVOID res = VirtualAlloc(addr, commited_region_size, MEM_COMMIT, PAGE_READWRITE);
		if (static_cast<char *>(res))
			return true;
		return false;
	}

	void rmReservedPages ()
	{
		if (m_region_base_addr)
		{
			qDebug("MEMORY: virtualfree at base=0x%x sz=%u", m_region_base_addr, m_region_capacity);
			VirtualFree(m_region_base_addr, 0, MEM_RELEASE);
			m_region_base_addr = nullptr;
		}
		//@TODO: MEM_RESET
	}

	constexpr size_t getPageSize () const { return 64 * 1024; }
	constexpr size_t getPageCount () const { return 16; }

	bool mkStorage (size_t reserved_region_size)
	{
		if (mkReservedPages(reserved_region_size))
		{
			if (mkCommitPages(m_region_base_addr, getPageCount() * getPageSize()))
			{
				m_region_size = getPageCount() * getPageSize();
				return true;
			}
		}
		return false;
	}

	void releaseStorage ()
	{
		rmReservedPages();
	}

	bool resizeStorage (size_t n)
	{
		size_t const curr_sz = m_region_size;
		size_t const commit_sz = n - curr_sz;
		if (curr_sz + commit_sz <= m_region_capacity && mkCommitPages(m_region_base_addr + curr_sz, commit_sz))
		{
			m_region_size += commit_sz;		
// 		char buff[256];
// 		snprintf(buff, 256, "# resize %d  base=0x%08x curr=%d commit_sz=%d\n", n, m_region_base_addr, curr_sz, commit_sz);
// 		OutputDebugStringA(buff);
			return true;
		}
		return false;
	}

	size_t calcNextSize () const 
	{
		size_t const sz = m_region_size + getPageCount() * getPageSize();
		return sz;
	}
};


