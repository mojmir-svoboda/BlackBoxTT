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

inline void showWindow (HWND hwnd, bool show)
{
	::ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
}

// inline void /*Workspaces::*/SwitchToWindow(HWND hwnd_app)
// {
// 	SwitchToThisWindow(hwnd_app, 1);
// 	// 		HWND hwnd = GetLastActivePopup(GetRootWindow(hwnd_app));
// 	// 		if (have_imp(pSwitchToThisWindow)) {
// 	// 			// this one also restores the window, if it's iconic:
// 	// 			pSwitchToThisWindow(hwnd, 1);
// 	// 		}
// 	// 		else {
// 	// 			SetForegroundWindow(hwnd);
// 	// 			if (IsIconic(hwnd))
// 	// 				send_syscommand(hwnd, SC_RESTORE);
// 	// 		}
// }
// 
// inline void /*Workspaces::*/WS_BringToFront(HWND hwnd, bool to_current)
// {
// 	int windesk;
// 
// 	//CleanTasks();
// 
// 	// 		windesk = vwm_get_desk(hwnd);
// 	// 		if (windesk != currentScreen)
// 	// 		{
// 	// 			if (false == to_current)
// 	// 				switchToDesktop(windesk);
// 	// 			else
// 	// 				setDesktop(hwnd, currentScreen, false);
// 	// 		}
// 	SwitchToWindow(hwnd);
// }
// 
// inline bool /*Workspaces::*/FocusTopWindow()
// {
// 	// 		HWND hw = get_top_window(currentScreen);
// 	// 		if (hw) {
// 	// 			SwitchToWindow(hw);
// 	// 			return true;
// 	// 		}
// 	// 		SwitchToBBWnd();
// 	return false;
// }

inline void focusWindow (HWND hwnd)
{
	// @TODO: if curr_ws != hwnd->ws ==> switch
	SwitchToThisWindow(hwnd, 1);
}

inline void showInFromTaskBar (HWND hwnd, bool show)
{
	if (show)
	{
		long style = ::GetWindowLong(hwnd, GWL_STYLE);
		style &= ~(WS_VISIBLE);    // this works - window become invisible 

		style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
		style &= ~(WS_EX_APPWINDOW);

		::ShowWindow(hwnd, SW_HIDE); // hide the window
		::SetWindowLong(hwnd, GWL_STYLE, style); // set the style
		::ShowWindow(hwnd, SW_SHOW); // show the window for the new style to come into effect
		::ShowWindow(hwnd, SW_HIDE); // hide the window so we can't see it
	}
	else
	{
		long style = ::GetWindowLong(hwnd, GWL_STYLE);
		style &= ~(WS_VISIBLE);    // this works - window become invisible 

		style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
		style &= ~(WS_EX_APPWINDOW);

		::ShowWindow(hwnd, SW_HIDE); // hide the window
		::SetWindowLong(hwnd, GWL_STYLE, style); // set the style
		::ShowWindow(hwnd, SW_SHOW); // show the window for the new style to come into effect
		::ShowWindow(hwnd, SW_HIDE); // hide the window so we can't see it
	}
}


}
