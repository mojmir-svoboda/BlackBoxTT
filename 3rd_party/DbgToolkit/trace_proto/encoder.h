#pragma once
#include <OCTET_STRING.h>
#define USE_CXX_ALLOCATOR 1
#include <asn_allocator.h>
#include "membuffer.h"

namespace asn1 {

	struct Asn1StackAllocator : Allocator, MemBuffer<StackBuffer<1024>>
	{
		virtual void * MALLOC(size_t count) override { return Malloc(count); }
		virtual void * CALLOC(size_t count, size_t size) override { return Calloc(count, size); }
		virtual void * REALLOC(void * old_mem, size_t old_size, size_t new_size) override
		{
			if (old_mem == nullptr)
				return MALLOC(new_size);
			else
			{
				void * new_mem = MALLOC(new_size);
				if (old_size)
				{
					memcpy(new_mem, old_mem, old_size);
					FREEMEM(old_mem);
				}
				return new_mem;
			}
			return nullptr;
		}
		virtual void  FREEMEM(void * mem) override { }
	};

	inline OCTET_STRING_t mkOctetStringRaw (char const * raw_str, size_t ln)
	{
		OCTET_STRING_t ostr;
		ostr.buf = const_cast<unsigned char *>(reinterpret_cast<unsigned char const *>(raw_str));
		ostr.size = static_cast<int>(ln);
		return ostr;
	}

	inline OCTET_STRING_t mkOctetString (char const * raw_str)
	{
		OCTET_STRING_t ostr;
		ostr.buf = const_cast<unsigned char *>(reinterpret_cast<unsigned char const *>(raw_str));
		ostr.size = static_cast<int>(strlen(raw_str));
		return ostr;
	}
}

