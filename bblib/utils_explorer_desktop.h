#pragma once
#include "platform_win.h"
#include "bbstring.h"
#include "CommCtrl.h"
#include <Assert/Assert.h>

namespace bb {

	BOOL CALLBACK enumWindowsProc (HWND hwnd, LPARAM lParam)
	{
		wchar_t buffer[128];
		int const written = ::GetClassName(hwnd, buffer, 128);
		if (written && wcscmp(buffer, L"SHELLDLL_DefView") == 0)
		{
			wchar_t buffer2[128];
			int const written2 = ::GetWindowText(hwnd, buffer2, 128);
			if (written2 == 0)
			{
				*(HWND*)lParam = hwnd;
				return FALSE;
			}
		}
		return TRUE;
	}
	HWND getDesktopParentHandle ()
	{
		HWND hWnd = NULL;
		EnumChildWindows(::GetDesktopWindow(), enumWindowsProc, (LPARAM)&hWnd);
		return hWnd;
	}
	HWND getDesktopHandle ()
	{
		HWND hShellWnd = GetShellWindow();
		//HWND  hShellWnd = FindWindow(L"Progman", L"Program Manager"));
		HWND hDefView = FindWindowEx(hShellWnd, NULL, L"SHELLDLL_DefView", NULL);
		HWND folderView = FindWindowEx(hDefView, NULL, L"SysListView32", NULL);
		return folderView;
	}
	HWND getDesktopHandleBruteForce ()
	{
		// workaround if the above does not work (can happen when progman crashes)
		HWND const hDefView = getDesktopParentHandle(); //FindWindowEx(shell_wnd, nullptr, L"SHELLDLL_DefView", nullptr);
		HWND const folderView = FindWindowEx(hDefView, NULL, L"SysListView32", L"FolderView");
		return folderView;
	}
	bool isDesktopHandle (HWND hwnd)
	{
		// classic method
		// gee, diz shite no good
// 		if (HWND d = getDesktopHandle())
// 		{
// 			return hwnd == d;
// 		}

		if (HWND explorerFolderView = getDesktopHandleBruteForce())
		{
			if (hwnd == explorerFolderView)
				return true;
		}

		HWND const desktop_window = ::GetDesktopWindow();
		return hwnd == desktop_window;
	}

	// begin: following code is from http://www.codeguru.com/cpp/misc/misc/article.php/c3807/Obtaining-Icon-Positions.htm
	// Posted by Jeroen-bart Engelen, modified to suit my needs (mojmir)
	void * allocMemInForeignProcess (HANDLE process, unsigned long size)
	{
		void * ptr = ::VirtualAllocEx(process, NULL, size, MEM_COMMIT, PAGE_READWRITE);
		return ptr;
	}
	bool freeMemInForeignProcess (HANDLE process, void * ptr)
	{
		return ::VirtualFreeEx(process, ptr, 0, MEM_RELEASE) == TRUE;
	}
	bool readFromForeignProcessMemory (HANDLE process, void * ptr, void * target, unsigned long size)
	{
		return ::ReadProcessMemory(process, ptr, target, size, nullptr) == TRUE;
	}
	bool WriteToForeignProcessMemory (HANDLE process, void * ptr, void * src, unsigned long size)
	{
		return ::WriteProcessMemory(process, ptr, src, size, nullptr) == TRUE;
	}
	HANDLE findExlorerProcess (HWND slave_wnd)
	{ 
		// Thanks to mr_williams@rave.ch who pointed me to GetWindowThreadProcessId(), that makes this function waaaaaaaaaaaaay shorter.
		// Get the PID based on a HWND. This is the good stuff. You wouldn't believe the long and difficult function I had to write before I heard of this simple API call.
		DWORD explorer_pid = 0;
		::GetWindowThreadProcessId(slave_wnd, &explorer_pid);
		// Get a process handle which we need for the shared memory functions.
		HANDLE proc = ::OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, explorer_pid);
		return proc;
	}

	int getDesktopIconCount (HWND hwndSysListView32)
	{
		int const iconcount = ListView_GetItemCount(hwndSysListView32);
		return iconcount;
	}
	// Query the shell listview of the desktop for the icons that are on the desktop. Since we're talking to a foreign process (explorer.exe), we need to use shared memory to get the data.
	bool getDesktopIcons (HWND hwndSysListView32, size_t icon_count, RECT * results)
	{
		bool error = false;
		if (HANDLE explorer = findExlorerProcess(hwndSysListView32)) // Get the PID of the process that houses the listview, i.e.: Explorer.exe
		{
			for (int i = 0; i < icon_count; ++i)
			{
				if (void * ipc_icon_rect = allocMemInForeignProcess(explorer, sizeof(RECT))) // allocate the shared memory buffers to use in our little IPC.
				{
					BOOL const res = ::SendMessage(hwndSysListView32, LVM_GETITEMRECT, i, reinterpret_cast<LPARAM>(ipc_icon_rect));
					if (res == TRUE) 
					{
						RECT icon_rc = { };
						readFromForeignProcessMemory(explorer, ipc_icon_rect, &icon_rc, sizeof(RECT)); // Get the data from the shared memory
						results[i] = icon_rc;
					}
					else
					{
						Assert(0); // something went wrong, cannot get icon rect
						error = true;
					}

					freeMemInForeignProcess(explorer, ipc_icon_rect);
					ipc_icon_rect = nullptr;
				}
			}

			::CloseHandle(explorer);
		}
		return !error;
	}
	// end of: following code is from http://www.codeguru.com/cpp/misc/misc/article.php/c3807/Obtaining-Icon-Positions.htm

	bool clickedOnDesktopIcon (POINT p)
	{
 		HWND  hwndSysListView32 = getDesktopHandleBruteForce();
		int const count = getDesktopIconCount(hwndSysListView32);

		RECT * const rects = reinterpret_cast<RECT *>(alloca(sizeof(RECT) * count));
		if (getDesktopIcons(hwndSysListView32, count, rects))
		{
			for (int i = 0; i < count; ++i)
			{
				RECT const & r = rects[i];
				if (::PtInRect(&r, p))
					return true;
			}
		}
		return false;
	}
}

