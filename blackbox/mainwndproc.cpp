#include <platform_win.h>
#include <hooks/shellhook.h>
#include <hooks/trayhook.h>
#include <vector>
#include <SpinLock.h>
#include <utils_window.h>
#include <Tasks.h>
#include <BlackBox.h>
#include <Gfx/Gui.h>
#include <utils_paths.h>

BOOL CALLBACK taskEnumProc (HWND hwnd, LPARAM lParam)
{
	if (bb::isAppWindow(hwnd))
	{
		bb::BlackBox::Instance().GetTasks().AddTask(hwnd);
	}
	return TRUE;
}

#include <strsafe.h>
void initShellHook32on64 (HWND bb_hwnd)
{
	TCHAR path[1024];
	bb::getExePath(path, 1024);

	//TCHAR szModule[MAX_PATH];
	TCHAR szCommand[MAX_PATH * 2];

	TCHAR mod[MAX_PATH * 2];
	_snwprintf(mod, MAX_PATH * 2, L"%s\\blackbox32.exe", path);
	//DWORD cch = GetModuleFileName(NULL, szModule, MAX_PATH);
	//if (cch > 0 && cch < MAX_PATH &&
	if (SUCCEEDED(StringCchPrintf(szCommand, MAX_PATH * 2, TEXT("\"%s\" %I64d"), mod, (INT64)(INT_PTR)bb_hwnd)))
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		BOOL bRes = CreateProcess(mod, szCommand, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
		if (bRes)
			WaitForInputIdle(pi.hProcess, INFINITE);
	}
}


LRESULT CALLBACK mainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bb::BlackBox & bb = bb::BlackBox::Instance();

	switch (uMsg)
	{
		case WM_CREATE:
		{
			if (!bb.m_cmdLine.NoTaskHook())
			{
				initShellHook(hwnd);
				initShellHook32on64(hwnd);
			}
			if (!bb.m_cmdLine.NoTrayHook())
			{
				initTrayHook(hwnd);
			}
			return 0;
		}
		case WM_DESTROY:
			doneShellHook();
			PostQuitMessage(0);
			break;

		//====================
		case WM_ENDSESSION:
			//if (wParam)
			//	shutdown_blackbox();
			break;
		case WM_QUERYENDSESSION:
			//getWorkspaces().GatherWindows();
			return TRUE;

		case WM_ACTIVATEAPP:
			break;

		case WM_SIZE:
		{
			unsigned w = (UINT)LOWORD(lParam);
			unsigned h = (UINT)HIWORD(lParam);
			return 0;
		}
		case WM_DISPLAYCHANGE:
			break;

		case WM_CLOSE:
			break;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;

		default:
		{
			if (uMsg == g_WM_ShellHook)
			{
				LRESULT const res = bb::BlackBox::Instance().GetTasks().UpdateFromShellHook(wParam, lParam);
				return res;
			}

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return 0;
}

