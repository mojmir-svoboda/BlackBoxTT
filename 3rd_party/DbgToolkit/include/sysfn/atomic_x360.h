#pragma once
//#define WIN32_LEAN_AND_MEAN
#include <xtl.h>

namespace sys {

	typedef LONG atomic32_t;

	inline atomic32_t atomic_get32 (atomic32_t volatile const * val)
	{
#if defined __MINGW32__
#else
		MemoryBarrier();
#endif
		return *val;
	}

	inline atomic32_t atomic_cas32 (atomic32_t volatile * mem, atomic32_t oldv, atomic32_t newv)
	{
		return InterlockedCompareExchange(mem, newv, oldv);
	}

	inline atomic32_t atomic_faa32 (atomic32_t volatile * mem, atomic32_t val)
	{
		return InterlockedExchangeAdd(mem, val);
	}

	inline void lwsync ()
	{
		__lwsync();
	}
}

