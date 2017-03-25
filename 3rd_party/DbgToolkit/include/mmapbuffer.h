#pragma once
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <fs.h>
#include <string>
#include <cassert>
#include <boost/filesystem.hpp>
#include <windows.h>

inline bool mmap_region (boost::interprocess::file_mapping & fmap, boost::interprocess::mapped_region & freg, void * addr)
{
	freg = boost::interprocess::mapped_region(fmap, boost::interprocess::read_write, 0, 0, addr);
	return freg.get_size() > 0;
}

struct MMapHeader
{
	uint16_t m_version;
	uint16_t m_reserved;
	uint64_t m_end_offset;
};

struct MMapBuffer
{
	fs::path m_file;
	boost::interprocess::file_mapping m_map;
	boost::interprocess::mapped_region m_reg;
	MMapHeader * m_hdr;

	MMapBuffer () : m_hdr(nullptr) { }

	~MMapBuffer ()
	{
		releaseStorage();

		try {
			if (fs::exists(m_file))
				fs::remove(m_file);
		}
		catch (std::exception const &)
		{
			//OutputDebugStringA(e.what());
		}
		catch (...)
		{ }
	}

	char * begin () { return static_cast<char *>(m_reg.get_address()) + sizeof(MMapHeader); }
	char const * begin () const { return static_cast<char const *>(m_reg.get_address()) + sizeof(MMapHeader); }
	char * end () { return begin() + m_hdr->m_end_offset; }
	char const * end () const { return begin() + m_hdr->m_end_offset; }
	size_t capacity () const { return m_reg.get_size() - sizeof(MMapHeader); }
	size_t shiftEnd (size_t n) {  return m_hdr->m_end_offset += n; }

	bool mkTempStorage (size_t n, size_t reserverd_region_size)
	{
		boost::filesystem::path p = boost::filesystem::unique_path();
#if _MSC_VER < 1900
#else
		fs::path tmp_path = fs::temp_directory_path();
		m_file = tmp_path / p.native();
#endif
		return getStorageFromFile(m_file, n);
	}

	bool getStorageFromFile (fs::path const & fname, size_t n)
	{
		try
		{
			if (!fs::exists(fname))
			{
				{
					std::filebuf fbuf;
					fbuf.open(fname.native().c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
					fbuf.pubseekoff(n - 1, std::ios_base::beg);
					fbuf.sputc(0);
					fbuf.close();
				}
				m_map = boost::interprocess::file_mapping(fname.native().c_str(), boost::interprocess::read_write);
				if (mmap_region(m_map, m_reg, nullptr))
				{
					char * hdr_mem = static_cast<char *>(m_reg.get_address());
					m_hdr = new (hdr_mem) MMapHeader;
					m_hdr->m_version = 0x0101;
					m_file = fname;

					return true;
				}
				Q_ASSERT(0);
				return false;
			}
			else
			{
				m_map = boost::interprocess::file_mapping(fname.native().c_str(), boost::interprocess::read_write);
				if (mmap_region(m_map, m_reg, nullptr))
				{
					m_hdr = reinterpret_cast<MMapHeader *>(m_reg.get_address());
					m_file = fname;
					return true;
				}
				Q_ASSERT(0);
				return false;
			}
		}
		catch (const boost::interprocess::interprocess_exception & e)
		{
			Q_ASSERT(0);
			qDebug(e.what());
			return false;
		}
		return true;
	}

	void releaseStorage ()
	{
		m_reg = boost::interprocess::mapped_region();
		m_map = boost::interprocess::file_mapping();
	}

	bool resizeStorage (size_t n)
	{
		void const * old = m_reg.get_address();
		releaseStorage();
		fs::resize_file(m_file, n);
		getStorageFromFile(m_file, n);
		void const * nevv = m_reg.get_address();
		if (old != nevv)
		{
			char buff[256];
			DebugBreak(); // uh-oh
			snprintf(buff, 256, "# resize %d  0x%08x -> 0x%08x\n", n, old, nevv);
			OutputDebugStringA(buff);
		}
		return true;
	}

	size_t calcNextSize () const 
	{
		//size_t const grow_cst = 1024 * 1024;
		size_t const grow_cst = 1024;
		size_t const sz = m_reg.get_size() + grow_cst;
		return sz;
	}
};


