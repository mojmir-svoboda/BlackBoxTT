#pragma once
#include <trace_proto/decoder_alloc.h>

void * Asn1Allocator::CALLOC (size_t count, size_t size)
{
	return Calloc(count, size);
}

void * Asn1Allocator::MALLOC (size_t size)
{
	return Malloc(size);
}

void * Asn1Allocator::REALLOC (void * old_mem, size_t old_size, size_t size)
{
	if (old_mem == nullptr)
		return MALLOC(size);
	else
	{
		void * new_mem = MALLOC(size);
		if (old_size)
		{
			memcpy(new_mem, old_mem, old_size);
			FREEMEM(old_mem);
		}
		return new_mem;
	}
	return nullptr;
}

void  Asn1Allocator::FREEMEM (void * mem)
{ }

// #include <trace_proto/decoder_alloc.h>
// 
// Asn1Alloc g_Asn1Alloc;
// 
// extern "C" {
// 	void * CALLOC (size_t count, size_t size)
// 	{
// 		return g_Asn1Alloc.Asn1DecoderCalloc(count, size);
// 	}
// 	void * MALLOC (size_t size)
// 	{
// 		return g_Asn1Alloc.Asn1DecoderMalloc(size);;
// 	}
// 	void * REALLOC (void * mem, size_t oldsize, size_t size)
// 	{
// 		return g_Asn1Alloc.Asn1DecoderRealloc(mem, oldsize, size);
// 	}
// 	void	FREEMEM (void * mem)
// 	{
// 	}
// }
