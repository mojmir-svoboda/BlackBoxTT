#pragma once
#include <dwmapi.h>
#include <string.h>

namespace bb {

	inline bool isDwmEnabled ()
	{
		BOOL on = FALSE;
		if (S_OK == ::DwmIsCompositionEnabled(&on))
			return on == TRUE;
		return false;
	}

	inline bool isUWPWindowCloaked (HWND hwnd)
	{
		int cloaked = 0;
		HRESULT hr = ::DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
		if (hr != S_OK)
			return false;
		return cloaked != 0;
	}

	inline bool isUWPWindow (HWND hwnd)
	{
		TCHAR clss[256];
		::GetClassName(hwnd, clss, 256);
		bool is_uwp = false;
		is_uwp |= 0 == wcsncmp(clss, L"ApplicationFrameWindow", 256);
		is_uwp |= 0 == wcsncmp(clss, L"Windows.UI.Core.CoreWindow", 256);
		return is_uwp;
	}

}
