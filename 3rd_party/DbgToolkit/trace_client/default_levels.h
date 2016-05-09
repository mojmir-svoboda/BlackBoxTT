#pragma once

namespace trace {

	typedef unsigned int level_t;

	// @NOTE: all the level types are in .inc file
#define DRY(a,b) static level_t const a = b;
#	include "default_levels.inc"
#undef DRY

}
