#pragma once
#include <platform_win.h>

#if defined (shellhook_EXPORTS) || defined (shellhook32_EXPORTS)
#	define SHELLHOOK_API __declspec(dllexport)
#else
#	define SHELLHOOK_API __declspec(dllimport)
#endif

extern SHELLHOOK_API unsigned g_WM_ShellHook;
extern SHELLHOOK_API HHOOK g_shellHook;

SHELLHOOK_API void initShellHook (HWND bb_hwnd);
SHELLHOOK_API void doneShellHook ();
LRESULT CALLBACK taskManagerHookProc (int code, WPARAM wParam, LPARAM lParam);

