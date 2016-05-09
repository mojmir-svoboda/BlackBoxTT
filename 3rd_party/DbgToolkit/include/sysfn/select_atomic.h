#pragma once

#if defined WIN32 || defined WIN64
#		include "atomic_win.h"
#elif defined _XBOX
#		include "atomic_x360.h"
#else
#		include "atomic_gcc.h"
#endif

