#pragma once
#include <cassert>
#define USE_CXX_ALLOCATOR 1
#include <asn_allocator.h>
#include "membuffer.h"

struct Asn1Allocator : Allocator, MemBuffer<HeapBuffer<16384>>
{
	virtual void * MALLOC (size_t count) override;
	virtual void * CALLOC (size_t count, size_t size) override;
	virtual void * REALLOC (void * mem, size_t old_size, size_t size) override;
	//void * reallocate (void * mem, size_t size);
	virtual void  FREEMEM (void * mem) override;
};

