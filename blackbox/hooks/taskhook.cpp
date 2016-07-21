#include "taskhook.h"
#include <cstdio>
#include <cstdint>
#include <bblib/utils_paths.h>

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

TASKHOOK_API bool initTaskHook (HWND bb_hwnd, unsigned wm_taskhook)
{
	g_bbHwnd = bb_hwnd;
	g_WM_TaskHook = wm_taskhook;
	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%ws.dll", c_taskHookName);
	HINSTANCE taskhook_hinst = ::GetModuleHandle(tmp);

	// @TODO: use WH_CBT
	g_taskHook = ::SetWindowsHookEx(WH_SHELL, (HOOKPROC)taskManagerHookProc, taskhook_hinst, 0);

	if (!g_taskHook)
	{
		uint32_t const err = GetLastError();
		return false;
	}
	return true;
}

TASKHOOK_API bool initTaskHook32 (HWND bb_hwnd, unsigned wm_taskhook)
{
	g_bbHwnd = bb_hwnd;
	g_WM_TaskHook = wm_taskhook;
	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%ws.dll", c_taskHook32Name);
	HINSTANCE taskhook_hinst = GetModuleHandle(tmp);
	g_taskHook = SetWindowsHookEx(WH_SHELL, (HOOKPROC)taskManagerHookProc, taskhook_hinst, 0);
	//g_taskHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)taskManagerHookProc32, taskhook_hinst, 0);

	if (!g_taskHook)
	{
		uint32_t const err = GetLastError();
		return false;
	}
	return true;
}

TASKHOOK_API void doneTaskHook ()
{
	UnhookWindowsHookEx(g_taskHook);
	g_taskHook = nullptr;
}


#include <strsafe.h>
bool initTaskHook32on64(wchar_t const * bb32name, HWND bb_hwnd, unsigned wm, HANDLE job, bool pcs_in_job)
{
	TCHAR path[1024];
	bb::getExePath(path, 1024);

	TCHAR bb32_path[MAX_PATH * 2];
	if (!bb::joinPath(path, bb32name, bb32_path, MAX_PATH * 2))
	{
		//TRACE_MSG(LL_ERROR, CTX_BB | CTX_HOOK | CTX_INIT, "Cannot join dir=%s with fname=%s", path, bb32name);
		return false;
	}

	TCHAR szCommand[MAX_PATH * 2];
	if (SUCCEEDED(StringCchPrintf(szCommand, MAX_PATH * 2, TEXT("\"%s\" -b %I64d -w %i"), bb32_path, (INT64)(INT_PTR)bb_hwnd, (int)wm)))
	{
		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);
		DWORD const dwCreationFlags = job && pcs_in_job ? CREATE_BREAKAWAY_FROM_JOB : 0;

		if (::CreateProcess(bb32_path, szCommand, NULL, NULL, TRUE, dwCreationFlags, NULL, NULL, &si, &pi))
		{
			if (job)
			{
				if (false == ::AssignProcessToJobObject(job, pi.hProcess))
				{
					//TRACE_MSG(LL_ERROR, CTX_BB | CTX_HOOK | CTX_INIT, "SetInformationJobObject failed");
				}
			}

			WaitForInputIdle(pi.hProcess, INFINITE);
			return true;
		}
	}
	//TRACE_MSG(LL_ERROR, CTX_BB | CTX_HOOK | CTX_INIT, "Cannot run command: %s", szCommand);
	return false;
}
