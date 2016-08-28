#pragma once
#include <platform_win.h>
#include <utils_dwm.h>

namespace bb {

inline void getWindowText (HWND hwnd, wchar_t * str, size_t n)
{
	str[0] = '\0';
	::GetWindowTextW(hwnd, str, n);
}

inline bool isAppWindow (HWND hwnd)
{
	if (!IsWindow(hwnd))
		return false;

	//dbg
	//TCHAR name[256] = {0}; getWindowText(hwnd, name, 256);
	//TCHAR clname[256] = { 0 }; ::GetClassName(hwnd, clname, 256);

	if (isUWPWindow(hwnd))
	{
		if (isUWPWindowCloaked(hwnd))
			return false;

		if (::IsWindowVisible(hwnd))
			return true;
		else
			return false;
	}

	LONG_PTR const style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
	if (WS_VISIBLE != (style & (WS_CHILD | WS_VISIBLE | WS_DISABLED)))
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

inline void showWindow (HWND hwnd, bool show)
{
	::ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
}

inline void focusWindow (HWND hwnd)
{
	// @TODO: if curr_ws != hwnd->ws ==> switch
	SwitchToThisWindow(hwnd, 1);
}

inline void showInFromTaskBar (HWND hwnd, bool show)
{
	if (show)
	{
		LONG_PTR const flags = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		if (WS_EX_TOOLWINDOW & flags)
		{
			::ShowWindow(hwnd, SW_HIDE);
			::SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
			::ShowWindow(hwnd, SW_SHOW);
		}

// 		long style = ::GetWindowLong(hwnd, GWL_STYLE);
// 		style &= ~(WS_VISIBLE);    // this works - window become invisible 
// 
// 		style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
// 		style &= ~(WS_EX_APPWINDOW);
// 
// 		::ShowWindow(hwnd, SW_HIDE); // hide the window
// 		::SetWindowLong(hwnd, GWL_STYLE, style); // set the style
// 		::ShowWindow(hwnd, SW_SHOW); // show the window for the new style to come into effect
// 		::ShowWindow(hwnd, SW_HIDE); // hide the window so we can't see it
	}
	else
	{
		::ShowWindow(hwnd, SW_HIDE);
		::SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		::ShowWindow(hwnd, SW_SHOW);
// 		long style = ::GetWindowLong(hwnd, GWL_STYLE);
// 		style &= ~(WS_VISIBLE);    // this works - window become invisible 
// 
// 		style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
// 		style &= ~(WS_EX_APPWINDOW);
// 
// 		::ShowWindow(hwnd, SW_HIDE); // hide the window
// 		::SetWindowLong(hwnd, GWL_STYLE, style); // set the style
// 		::ShowWindow(hwnd, SW_SHOW); // show the window for the new style to come into effect
// 		::ShowWindow(hwnd, SW_HIDE); // hide the window so we can't see it
	}
}


}
