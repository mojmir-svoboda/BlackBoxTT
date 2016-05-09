#pragma once
#include <cassert>
#include <vector>
#include <array>
#include <cstdint>

template<size_t NBytes>
struct StackBuffer
{
	StackBuffer() : m_end_offset(0) { }

	size_t m_end_offset;
	std::array<char, NBytes> m_storage;

	char * begin() { return &m_storage[0]; }
	char const * begin() const { return &m_storage[0]; }
	char * end() { return begin() + m_end_offset; }
	char const * end() const { return begin() + m_end_offset; }
	size_t capacity() const { return m_storage.size(); }
	size_t shiftEnd(size_t n) { return m_end_offset += n; }
	void reset() { m_end_offset = 0; }
	size_t available() const { return m_storage.size() - m_end_offset; }

	bool resizeStorage(size_t n) { return true; }
	size_t calcNextSize() const { return m_storage.size(); }
};

template<size_t NGrowBytes>
struct HeapBuffer
{
	HeapBuffer () : m_end_offset(0) { }

	size_t m_end_offset;
	std::vector<char> m_heap;

	char * begin () { return &m_heap[0]; }
	char const * begin () const { return &m_heap[0]; }
	char * end () { return begin() + m_end_offset; }
	char const * end () const { return begin() + m_end_offset; }
	size_t capacity () const { return m_heap.size(); }
	size_t shiftEnd (size_t n) {  return m_end_offset += n; }
	void reset () { m_end_offset = 0; }
	size_t available () const { return m_heap.size() - m_end_offset; }

	bool resizeStorage (size_t n)
	{
		std::vector<char> tmp(n);
		std::copy_n(m_heap.begin(), m_end_offset, tmp.begin());
		m_heap = std::move(tmp);
		return true;
	}
	size_t calcNextSize () const 
	{
		size_t const grow_cst = NGrowBytes;
		size_t const sz = capacity() + grow_cst;
		return sz;
	}
};

template<class Base>
struct MemBuffer : Base
{
	bool moveEnd (size_t n)
	{
		if (this->end() + n <= this->begin() + this->capacity())
		{
			this->shiftEnd(n);
			return true;
		}
		return false;
	}

	void * Calloc (size_t count, size_t size)
	{
		return Malloc(count * size);
	}
	void * Malloc (size_t size)
	{
		char * mem = this->end();
		if (moveEnd(size))
			return mem;
		size_t const next_sz = this->calcNextSize();
		this->resizeStorage(next_sz);
		char * mem2 = this->end();
		if (moveEnd(size))
			return mem2;
		return nullptr;
	}
	void * Realloc (void * mem, size_t size)
	{
		if (mem == 0)
			return Malloc(size);
		else
			assert(0);
		return nullptr;
	}
	void Free (void * mem) { }

	void Reset ()
	{
		this->reset();
	}
};


