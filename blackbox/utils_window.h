#pragma once
#include <platform_win.h>

namespace bb {

inline bool isAppWindow (HWND hwnd)
{
	if (!IsWindow(hwnd))
		return false;

	if (WS_VISIBLE != (::GetWindowLongPtr(hwnd, GWL_STYLE) & (WS_CHILD | WS_VISIBLE | WS_DISABLED)))
		return false;

	if (WS_EX_TOOLWINDOW == (::GetWindowLongPtr(hwnd, GWL_EXSTYLE) & (WS_EX_TOOLWINDOW | WS_EX_APPWINDOW)))
		return false;

	// If this hwnd has a parent, then only accept this window if the parent is not accepted
	HWND hParent = ::GetParent(hwnd);
	if (NULL == hParent)
		hParent = ::GetWindow(hwnd, GW_OWNER);

	if (hParent && isAppWindow(hParent))
		return false;

	if (0 == ::GetWindowTextLengthW(hwnd))
		return false;

	return true;
}


inline void getWindowText (HWND hwnd, std::wstring & str)
{
	WCHAR wbuf[1024];
	wbuf[0] = 0;
	::GetWindowTextW(hwnd, wbuf, 1024);
	std::wstring ws(wbuf);
	str = std::move(ws);
}

// update frame:
//        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

inline void removeWindowBorder (HWND hwnd)
{
	// thanks to author of Deborder (@TODO)
	if (LONG styles = ::GetWindowLong(hwnd, GWL_STYLE))
	{
		styles ^= WS_CAPTION;

		bool deflate = 0 == (styles & WS_CAPTION);
		RECT rc;
		GetWindowRect(hwnd, &rc);

		int const captionHeight = ::GetSystemMetrics(SM_CYCAPTION);
		int const borderWidth = ::GetSystemMetrics(SM_CXDLGFRAME);
		int const borderHeight = ::GetSystemMetrics(SM_CYDLGFRAME);

		if (deflate)
		{
			rc.left += borderWidth;
			rc.right -= borderWidth;
			rc.top += captionHeight + borderHeight;
			rc.bottom -= borderHeight;
		}
		else
		{
			rc.left -= borderWidth;
			rc.right += borderWidth;
			rc.top -= captionHeight + borderHeight;
			rc.bottom += borderHeight;
		}
		LONG const result = ::SetWindowLong(hwnd, GWL_STYLE, styles);
		if (0 != result)
			::SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
	}
}

}
