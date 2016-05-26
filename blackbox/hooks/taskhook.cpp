#include "taskhook.h"
#include <cstdio>
#include <cstdint>

wchar_t const * const c_taskHookName = L"taskhook";
wchar_t const * const c_taskHook32Name = L"taskhook32";

#pragma data_seg(".shared")
TASKHOOK_API HHOOK g_taskHook = 0;
HWND g_bbHwnd = 0;
TASKHOOK_API unsigned g_WM_TaskHook = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

LRESULT CALLBACK taskManagerHookProc (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		PostMessage(g_bbHwnd, g_WM_TaskHook, code, wParam);
	}
	return CallNextHookEx(g_taskHook, code, wParam, lParam);
}

TASKHOOK_API void initTaskHook (HWND bb_hwnd)
{
	g_bbHwnd = bb_hwnd;
	g_WM_TaskHook = ::RegisterWindowMessage(c_taskHookName);
	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%ws.dll", c_taskHookName);
	HINSTANCE taskhook_hinst = ::GetModuleHandle(tmp);

	// @TODO: use WH_CBT
	g_taskHook = ::SetWindowsHookEx(WH_SHELL, (HOOKPROC)taskManagerHookProc, taskhook_hinst, 0);
	//RegisterTaskHookWindow 

	if (!g_taskHook)
	{
		uint32_t const err = GetLastError();
	}
}


LRESULT CALLBACK taskManagerHookProc32 (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		::PostMessage(g_bbHwnd, g_WM_TaskHook, code, wParam);
	}
	return ::CallNextHookEx(g_taskHook, code, wParam, lParam);
}

TASKHOOK_API void initTaskHook32 (HWND bb_hwnd)
{
	g_bbHwnd = bb_hwnd;
	g_WM_TaskHook = ::RegisterWindowMessage(c_taskHook32Name);
	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%ws.dll", c_taskHook32Name);
	HINSTANCE taskhook_hinst = GetModuleHandle(tmp);
	g_taskHook = SetWindowsHookEx(WH_SHELL, (HOOKPROC)taskManagerHookProc, taskhook_hinst, 0);
	//g_taskHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)taskManagerHookProc32, taskhook_hinst, 0);

	if (!g_taskHook)
	{
		uint32_t const err = GetLastError();
	}
}


TASKHOOK_API void doneTaskHook ()
{
	UnhookWindowsHookEx(g_taskHook);
	g_taskHook = nullptr;
}
