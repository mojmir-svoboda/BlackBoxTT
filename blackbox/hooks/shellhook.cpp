#include "shellhook.h"
#include <cstdio>
#include <cstdint>

wchar_t const * const c_shellHookName = L"shellhook";
wchar_t const * const c_shellHook32Name = L"shellhook32";

#pragma data_seg(".shared")
SHELLHOOK_API HHOOK g_shellHook = 0;
HWND g_bbHwnd = 0;
SHELLHOOK_API unsigned g_WM_ShellHook = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

LRESULT CALLBACK taskManagerHookProc (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		PostMessage(g_bbHwnd, g_WM_ShellHook, code, wParam);
	}
	return CallNextHookEx(g_shellHook, code, wParam, lParam);
}

SHELLHOOK_API void initShellHook (HWND bb_hwnd)
{
	g_bbHwnd = bb_hwnd;
	g_WM_ShellHook = ::RegisterWindowMessage(c_shellHookName);
	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%ws.dll", c_shellHookName);
	HINSTANCE shellhook_hinst = ::GetModuleHandle(tmp);
	g_shellHook = ::SetWindowsHookEx(WH_SHELL, (HOOKPROC)taskManagerHookProc, shellhook_hinst, 0);
	//RegisterShellHookWindow 

	if (!g_shellHook)
	{
		uint32_t const err = GetLastError();
	}
}


LRESULT CALLBACK taskManagerHookProc32 (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		::PostMessage(g_bbHwnd, g_WM_ShellHook, code, wParam);
	}
	return ::CallNextHookEx(g_shellHook, code, wParam, lParam);
}

SHELLHOOK_API void initShellHook32 (HWND bb_hwnd)
{
	g_bbHwnd = bb_hwnd;
	g_WM_ShellHook = ::RegisterWindowMessage(c_shellHook32Name);
	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%ws.dll", c_shellHook32Name);
	HINSTANCE shellhook_hinst = GetModuleHandle(tmp);
	g_shellHook = SetWindowsHookEx(WH_SHELL, (HOOKPROC)taskManagerHookProc, shellhook_hinst, 0);
	//g_shellHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)taskManagerHookProc32, shellhook_hinst, 0);

	if (!g_shellHook)
	{
		uint32_t const err = GetLastError();
	}
}


SHELLHOOK_API void doneShellHook ()
{
	UnhookWindowsHookEx(g_shellHook);
	g_shellHook = nullptr;
}
