#pragma once

enum LogLevel : unsigned
{
	LL_VERBOSE,
	LL_DEBUG,
	LL_INFO,
	LL_WARNING,
	LL_ERROR,
	LL_FATAL,

	e_max_trace_level /// last item of enum
};

namespace trace {
	typedef unsigned level_t;
}
