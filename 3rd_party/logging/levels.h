#pragma once

namespace trace {
	typedef uint32_t level_t;
}

const trace::level_t LL_FATAL				=  (1u <<  0);
const trace::level_t LL_ERROR				=  (1u <<  1);
const trace::level_t LL_WARNING			=  (1u <<  2);
const trace::level_t LL_INFO				=  (1u <<  3);
const trace::level_t LL_DEBUG				=  (1u <<  4);
const trace::level_t LL_VERBOSE			=  (1u <<  5);
