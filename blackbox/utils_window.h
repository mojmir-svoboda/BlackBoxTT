#pragma once
#include <platform_win.h>
#include <utils_dwm.h>

namespace bb {

	inline void getWindowText (HWND hwnd, wchar_t * str, size_t n)
	{
		str[0] = '\0';
		::GetWindowTextW(hwnd, str, n);
	}

// BOOL IsAltTabWindow(HWND hwnd)
// {
// 	TITLEBARINFO ti;
// 	HWND hwndTry, hwndWalk = NULL;
// 
// 	if (!IsWindowVisible(hwnd))
// 		return FALSE;
// 
// 	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
// 	while (hwndTry != hwndWalk)
// 	{
// 		hwndWalk = hwndTry;
// 		hwndTry = GetLastActivePopup(hwndWalk);
// 		if (IsWindowVisible(hwndTry))
// 			break;
// 	}
// 	if (hwndWalk != hwnd)
// 		return FALSE;
// 
// 	// the following removes some task tray programs and "Program Manager"
// 	ti.cbSize = sizeof(ti);
// 	GetTitleBarInfo(hwnd, &ti);
// 	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
// 		return FALSE;
// 
// 	// Tool windows should not be displayed either, these do not appear in the
// 	// task bar.
// 	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
// 		return FALSE;
// 
// 	return TRUE;
//
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

// from bblean
inline LRESULT sendSysCommand (HWND hwnd, WPARAM SC_XXX)
{
	DWORD_PTR dwResult = 0;
	::SendMessageTimeout(hwnd, WM_SYSCOMMAND, SC_XXX, 0, SMTO_ABORTIFHUNG|SMTO_NORMAL, 1000, &dwResult);
	return dwResult;
}

inline void getRect (HWND hwnd, RECT * rp)
{
	GetWindowRect(hwnd, rp);
	if (WS_CHILD & GetWindowLongPtr(hwnd, GWL_STYLE))
	{
		HWND pw = GetParent(hwnd);
		ScreenToClient(pw, (LPPOINT)&rp->left);
		ScreenToClient(pw, (LPPOINT)&rp->right);
	}
}

inline bool checkForRestore (HWND hwnd)
{
	WINDOWPLACEMENT wp;

	if (FALSE == ::IsZoomed(hwnd))
		return false;
	sendSysCommand(hwnd, SC_RESTORE);

	// restore the default maxPos (necessary when it was V-max'd or H-max'd)
	wp.length = sizeof wp;
	::GetWindowPlacement(hwnd, &wp);
	wp.ptMaxPosition.x = wp.ptMaxPosition.y = -1;
	::SetWindowPlacement(hwnd, &wp);
	return true;
}

inline void windowSetPos (HWND hwnd, RECT rc)
{
	int const width = rc.right - rc.left;
	int const height = rc.bottom - rc.top;
	SetWindowPos(hwnd, NULL, rc.left, rc.top, width, height, SWP_NOZORDER|SWP_NOACTIVATE);
}

inline void growWindow (HWND hwnd, bool v)
{
	RECT r1, r2;

	if (checkForRestore(hwnd))
		return;
	getRect(hwnd, &r1);
	::LockWindowUpdate(hwnd);
	sendSysCommand(hwnd, SC_MAXIMIZE);
	getRect(hwnd, &r2);
	if (v)
		r1.top = r2.top, r1.bottom = r2.bottom;
	else
		r1.left = r2.left, r1.right = r2.right;
	windowSetPos(hwnd, r1);
	::LockWindowUpdate(NULL);
}

inline void maximizeWindow (HWND hwnd, bool vertical)
{
	LONG_PTR style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
	if (0 == (WS_MAXIMIZEBOX & style))
		return;

	RECT r1, r2;

// 	HMONITOR hmon = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
// 	MONITORINFO target;
// 	target.cbSize = sizeof(MONITORINFO);
// 	::GetMonitorInfo(hmon, &target);
// 
// 	getRect(hwnd, &r1);
// 	if (vertical)
// 	{
// 		r1.top = target.rcWork.top;
// 		r1.bottom = target.rcWork.bottom;
// 	}
// 	else
// 	{
// 		r1.left = target.rcWork.left;
// 		r1.right = target.rcWork.right;
// 	}

	if (checkForRestore(hwnd))
		return;
	getRect(hwnd, &r1);
	showWindow(hwnd, false);
	LockWindowUpdate(hwnd);
	sendSysCommand(hwnd, SC_MAXIMIZE);
	getRect(hwnd, &r2);
	if (vertical)
		r1.top = r2.top, r1.bottom = r2.bottom;
	else
		r1.left = r2.left, r1.right = r2.right;
	windowSetPos(hwnd, r1);
	LockWindowUpdate(NULL);
	showWindow(hwnd, true);
}

struct SecondMon
{
	int x0, y0, x1, y1;
	bool found;
};

inline BOOL CALLBACK MonitorEnumProc (HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX iMonitor;
	iMonitor.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &iMonitor);

	SecondMon * s = reinterpret_cast<SecondMon *>(dwData);
	if (s->x0 == 0 && s->y0 == 0)
	{
		if (iMonitor.rcMonitor.left == 0 && iMonitor.rcMonitor.top == 0)
			return true; // first monitor

		s->x0 = iMonitor.rcMonitor.left;
		s->y0 = iMonitor.rcMonitor.top;
		s->x1 = iMonitor.rcMonitor.right;
		s->y1 = iMonitor.rcMonitor.bottom;
		s->found = true;
		return false; // abort enum
	}
	return true;
}

inline bool createRoundedRect (HWND hwnd, int r, int b, int rnd, int cimmermann_correction_constant)
{
	HRGN hrgn = ::CreateRoundRectRgn(0, 0, r, b, rnd + cimmermann_correction_constant, rnd + cimmermann_correction_constant);
	::SetWindowRgn(hwnd, hrgn, TRUE);
	::SetPropW(hwnd, L"region", hrgn);
	return true;
}
inline bool destroyRoundedRect (HWND hwnd)
{
	::DeleteObject(::GetPropW(hwnd, L"region"));
	::RemovePropW(hwnd, L"region");
	return true;
}
inline void resizeWindowToContents (HWND hwnd, int x, int y, int maxx, int maxy, int rnd)
{
	if (x > maxx && y > maxy)
	{
		RECT r;
		GetWindowRect(hwnd, &r);
		int Width = r.left = r.right;
		int Height = r.bottom - r.top;

		DWORD dwStyle = ::GetWindowLongPtr(hwnd, GWL_STYLE);
		DWORD dwExStyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);

		RECT rc = { 0, 0, x, y };
		//::AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

		destroyRoundedRect(hwnd);
		::SetWindowPos(hwnd, NULL, 0, 0, rc.right + 24, rc.bottom, SWP_NOZORDER | SWP_NOMOVE);
		createRoundedRect(hwnd, rc.right + 24, rc.bottom, rnd, rnd);
	}
}

}
