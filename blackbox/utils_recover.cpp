#include "utils_recover.h"
#include <platform_win.h>
#include <Psapi.h>
#include <TlHelp32.h>

static BOOL CALLBACK recoverWindowEnumProc (HWND hwnd, LPARAM lParam);

namespace bb {

	int GetAppByWindow (HWND hwnd, wchar_t * name, size_t n)
	{
		name[0] = '\0';

		DWORD pid = 0;
		::GetWindowThreadProcessId(hwnd, &pid); // determine the process id of the window handle

		HANDLE const hPr0 = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		wchar_t modname[1024];
		if (hPr0)
		{
			HMODULE hMod = nullptr;
			DWORD cbNeeded = 0;
			if (::EnumProcessModules(hPr0, &hMod, sizeof(hMod), &cbNeeded))
			{
				::GetModuleBaseName(hPr0, hMod, modname, 1024);
			}
			::CloseHandle(hPr0);
		}
		else
		{
			// grab all the modules associated with the process
			HANDLE const hPr1 = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
			if (hPr1 != INVALID_HANDLE_VALUE)
			{
				DWORD_PTR base = (DWORD_PTR)-1;
				MODULEENTRY32 me;
				me.dwSize = sizeof(me);
				if (::Module32First(hPr1, &me))
				{
					do
					{
						if ((DWORD_PTR)me.modBaseAddr < base)
						{
							wcscpy(name, me.szModule);
							base = (DWORD_PTR)me.modBaseAddr;
							//if (hMod == me.hModule)
							if (base <= 0x00400000)
								break;
						}
					}
					while (::Module32Next(hPr1, &me));
				}
				//_strlwr(name);
				CloseHandle(hPr1);
			}
		}
		return wcslen(name);
	}

	struct RecoverEnumParams
	{
		ProcessWindows * m_data;
		size_t m_size;
		size_t m_capacity;
	};

	size_t enumerateProcessHandles (ProcessWindows * data, size_t data_sz)
	{
		RecoverEnumParams params = { data, 0, data_sz };
		::EnumWindows(recoverWindowEnumProc, (LPARAM)&params);
		return params.m_size;
	}

}

static BOOL CALLBACK recoverWindowEnumProc (HWND hwnd, LPARAM lParam)
{
	bb::RecoverEnumParams * params = reinterpret_cast<bb::RecoverEnumParams *>(lParam);

	wchar_t windowtext[512];
	RECT r;
	if (::IsWindowEnabled(hwnd)
		&& (::IsIconic(hwnd) || (GetClientRect(hwnd, &r) && r.right && r.bottom))
		&& GetWindowTextW(hwnd, windowtext, 512))
	{
		wchar_t appname[512];
		wchar_t classname[512];
		classname[0] = 0;
		GetClassName(hwnd, classname, 256);
		
		bb::GetAppByWindow(hwnd, appname, 512);

		if (params->m_size < params->m_capacity)
		{
			size_t const idx = params->m_size++;
			params->m_data[idx].m_hwnd = hwnd;
			params->m_data[idx].m_visible = FALSE != ::IsWindowVisible(hwnd);
			wcsncpy(params->m_data[idx].m_appName, appname, 512);
			wcsncpy(params->m_data[idx].m_caption, windowtext, 512);
		}
		else
			return FALSE;
	}
	return TRUE;
}