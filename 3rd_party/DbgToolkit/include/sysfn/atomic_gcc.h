#pragma once
#include <stdint.h>

namespace sys {

	typedef uint32_t atomic32_t;

	inline atomic32_t atomic_get32 (atomic32_t volatile const * val)
	{
		return *val;
	}

	inline atomic32_t atomic_cas32 (atomic32_t volatile * mem, atomic32_t oldv, atomic32_t newv)
	{
		return __sync_val_compare_and_swap(const_cast<atomic32_t *>(mem), oldv, newv);
	}

	inline atomic32_t atomic_faa32 (atomic32_t volatile * mem, atomic32_t val)
	{
		return __sync_fetch_and_add(const_cast<atomic32_t *>(mem), val);
	}

	inline void lwsync () { }
}

