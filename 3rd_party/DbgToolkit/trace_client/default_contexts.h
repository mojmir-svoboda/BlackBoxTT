#pragma once

namespace trace {

	typedef unsigned int context_t;

	// @NOTE: all the context types are in .inc file
#define DRY(a,b) static context_t const a = b;
#	include "default_contexts.inc"
#undef DRY
}
