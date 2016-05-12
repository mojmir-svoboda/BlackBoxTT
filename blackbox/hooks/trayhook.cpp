#include "trayhook.h"
#include <cstdio>
#include <cstdint>
#include <shellapi.h>

wchar_t const * const c_trayHookName = L"trayhook";

#pragma data_seg(".shared")
TRAYHOOK_API HHOOK g_trayHook = 0;
TRAYHOOK_API HHOOK g_trayHookW = 0;
TRAYHOOK_API TrayData * g_trayData = nullptr;
alignas(128) TRAYHOOK_API char g_trayDataRaw[sizeof(TrayData)] = { 0 };
HWND g_bbHwnd = 0;
HWND g_shellTrayWnd = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

void makeCopy (CWPSTRUCT * cwps)
{
	COPYDATASTRUCT * cds = reinterpret_cast<COPYDATASTRUCT *>(cwps->lParam);
	NOTIFYICONDATA const * const nic = (NOTIFYICONDATA *)(((BYTE *)cds->lpData) + 8);
	INT const cmd = *(INT *)(((BYTE *)cds->lpData) + 4);
	switch (cmd)
	{
		case NIM_ADD:
			g_trayData->AddOrModify(nic->uID, nic->hWnd, nic, cds);
			break;
		case NIM_MODIFY:
			g_trayData->AddOrModify(nic->uID, nic->hWnd, nic, cds);
			break;
		case NIM_DELETE:
			g_trayData->Delete(nic->uID, nic->hWnd);
			break;
		default:
			break;
	}
}

LRESULT CALLBACK trayHookProc (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		CWPSTRUCT * cwps = reinterpret_cast<CWPSTRUCT *>(lParam);
		//if (cwps->hwnd == g_shellTrayWnd && cwps->message == WM_COPYDATA)
		if (cwps->message == WM_COPYDATA)
		{
			COPYDATASTRUCT * cds = reinterpret_cast<COPYDATASTRUCT *>(cwps->lParam);
			if (cds->dwData == 1 || cds->dwData == 0) // if this data refers to a tray icon...
			{
				NOTIFYICONDATA const * const nicData = (NOTIFYICONDATA *)(((BYTE *)cds->lpData) + 8);
				INT const iTrayCmd = *(INT *)(((BYTE *)cds->lpData) + 4);

				if ((int)nicData->uID != -1)
				{
					makeCopy(cwps);
				}
			}
		}
	}
	return CallNextHookEx(g_trayHook, code, wParam, lParam);
}

LRESULT CALLBACK trayHookProcW (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		CWPSTRUCT * cwps = reinterpret_cast<CWPSTRUCT *>(lParam);
		//if (cwps->hwnd == g_shellTrayWnd && cwps->message == WM_COPYDATA)
		if (cwps->message == WM_COPYDATA)
		{
			COPYDATASTRUCT * cds = reinterpret_cast<COPYDATASTRUCT *>(cwps->lParam);
			if (cds->dwData == 1 || cds->dwData == 0) // if this data refers to a tray icon...
			{
				NOTIFYICONDATA const * const nicData = (NOTIFYICONDATA *)(((BYTE *)cds->lpData) + 8);
				INT const iTrayCmd = *(INT *)(((BYTE *)cds->lpData) + 4);

				if ((int)nicData->uID != -1)
				{
					makeCopy(cwps);
				}
			}
		}
	}
	return CallNextHookEx(g_trayHookW, code, wParam, lParam);
}

TRAYHOOK_API void initTrayHook (HWND bb_hwnd)
{
	g_trayData = new(g_trayDataRaw) TrayData;

	g_bbHwnd = bb_hwnd;
	g_shellTrayWnd = FindWindow(L"Shell_TrayWnd", NULL);

	wchar_t tmp[128];
	_snwprintf(tmp, 128, L"%s.dll", c_trayHookName); // @TODO CO TU MELO BEJT? PROC TEN  WARN?
	HINSTANCE trayhook_hinst = GetModuleHandle(tmp);
	g_trayHook = SetWindowsHookExA(WH_CALLWNDPROC, (HOOKPROC)trayHookProc, trayhook_hinst, 0);
	//g_trayHook = SetWindowsHookExA(WH_CALLWNDPROC, (HOOKPROC)trayHookProc, trayhook_hinst, 0);
	g_trayHookW = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)trayHookProcW, trayhook_hinst, 0);

	if (!g_trayHook)
	{
		uint32_t const err = GetLastError();
	}
	if (!g_trayHookW)
	{
		uint32_t const err = GetLastError();
	}
}

TRAYHOOK_API void doneTrayHook ()
{
	// destroy g_trayData;
	UnhookWindowsHookEx(g_trayHook);
	g_trayHook = nullptr;
}
