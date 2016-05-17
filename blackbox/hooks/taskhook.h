#pragma once
#include <platform_win.h>

#if defined (taskhook_EXPORTS) || defined (taskhook32_EXPORTS)
#	define TASKHOOK_API __declspec(dllexport)
#else
#	define TASKHOOK_API __declspec(dllimport)
#endif

extern TASKHOOK_API unsigned g_WM_TaskHook;
extern TASKHOOK_API HHOOK g_taskHook;

TASKHOOK_API void initTaskHook (HWND bb_hwnd);
TASKHOOK_API void doneTaskHook ();
LRESULT CALLBACK taskManagerHookProc (int code, WPARAM wParam, LPARAM lParam);

TASKHOOK_API void initTaskHook32 (HWND bb_hwnd);
LRESULT CALLBACK taskManagerHookProc32 (int code, WPARAM wParam, LPARAM lParam);
