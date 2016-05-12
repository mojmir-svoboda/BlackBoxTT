#pragma once
#include <dwmapi.h>

namespace bb {

	inline bool isDwmEnabled ()
	{
		BOOL on = FALSE;
		if (S_OK == ::DwmIsCompositionEnabled(&on))
			return on == TRUE;
		return false;
	}

}
