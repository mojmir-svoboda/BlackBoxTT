#pragma once

#ifdef	__cplusplus
	struct Allocator
	{
		virtual void * MALLOC (size_t count) = 0;
		virtual void * CALLOC (size_t count, size_t size) = 0;
		virtual void * REALLOC (void * mem, size_t old_size, size_t size) = 0;
		//void * reallocate (void * mem, size_t size);
		virtual void  FREEMEM (void * mem) = 0;
	};
#	define CXX_ALLOC_WRAP allocator->
#else
	typedef struct Allocator { int dummy; };
	#define CXX_ALLOC_WRAP
#endif
