#pragma once
#include <platform_win.h>

#if defined (taskhook_EXPORTS) || defined (taskhook32_EXPORTS)
#	define TASKHOOK_API __declspec(dllexport)
#else
#	define TASKHOOK_API __declspec(dllimport)
#endif

wchar_t const * const c_taskHookName = L"taskhook";
wchar_t const * const c_taskHook32Name = L"taskhook32";

TASKHOOK_API bool initTaskHook (HWND bb_hwnd, unsigned wm_msg);
TASKHOOK_API void doneTaskHook ();
LRESULT CALLBACK taskManagerHookProc (int code, WPARAM wParam, LPARAM lParam);

TASKHOOK_API bool initTaskHook32 (HWND bb_hwnd, unsigned wm_msg);
TASKHOOK_API bool initTaskHook32on64 (wchar_t const * bb32name, HWND bb_hwnd, unsigned wm, HANDLE job, bool pcs_in_job);
