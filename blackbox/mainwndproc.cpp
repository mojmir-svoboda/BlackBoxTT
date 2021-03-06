#include <platform_win.h>
#if defined(USE_HOOKS)
#	include <hooks/taskhook.h>
#	include <hooks/trayhook.h>
#endif
#include <blackbox/common.h>
#include <vector>
#include <SpinLock.h>
#include <utils_window.h>
#include <Tasks.h>
#include <BlackBox.h>
#include <utils_paths.h>

namespace bb {

LRESULT CALLBACK mainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bb::BlackBox & bb = bb::BlackBox::Instance();

	switch (uMsg)
	{
		case WM_CREATE:
		{
#if defined(USE_HOOKS)
			if (!bb.m_cmdLine.NoTaskHook())
			{
				initTaskHook(hwnd, bb.GetTaskHookWM());
				initTaskHook32on64(bb::BlackBox::s_blackbox32Name, hwnd, bb.GetTaskHook32on64WM(), bb.GetJob(), bb.GetInJob());
			}
			if (!bb.m_cmdLine.NoTrayHook())
			{
				initTrayHook(hwnd);
			}
#endif
			return 0;
		}
		case WM_DESTROY:
#if defined(USE_HOOKS)
			doneTaskHook();
#endif
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
			const bool handled = bb.HandleBroamMessage(hwnd, uMsg, wParam, lParam);
			if (handled)
			{
				return 0;
			}

			if (uMsg == bb.m_taskHookWM || uMsg == bb.m_taskHook32on64WM)
			{
				LRESULT const res = bb::BlackBox::Instance().GetTasks().UpdateFromTaskHook(wParam, lParam);
				return res;
			}

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return 0;
}

}