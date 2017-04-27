#include "BlackBox.h"
#include <blackbox/common.h>
#include <bblib/utils_paths.h>
#include "utils_window.h"

namespace bb {

	void BlackBox::Quit (uint32_t arg)
	{
		m_quit = true;
	}

	void BlackBox::LoadPlugin (bbstring const & plugin_id)
	{
		m_plugins.LoadPlugin(plugin_id);
	}
	void BlackBox::UnloadPlugin (bbstring const & plugin_id)
	{
		m_plugins.UnloadPlugin(plugin_id);
	}
	bool BlackBox::IsPluginLoaded (bbstring const & plugin_id) const
	{
		return m_plugins.IsPluginLoaded(plugin_id);
	}

	void BlackBox::CreateWidgetFromId (bbstring const & widget_id)
	{
		m_gfx->MkWidgetFromId(widget_id.c_str());
	}

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
		//HWND  hShellWnd = FindWindow(_T("Progman"), _T("Program Manager"));
		HWND hDefView = FindWindowEx(hShellWnd, NULL, _T("SHELLDLL_DefView"), NULL);
		HWND folderView = FindWindowEx(hDefView, NULL, _T("SysListView32"), NULL);
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
	// Posted by Jeroen-bart Engelen 
	void* AllocMemInForeignProcess(HANDLE process, unsigned long size) throw(...)
	{
		void *ptr = VirtualAllocEx(process, NULL, size, MEM_COMMIT, PAGE_READWRITE);
		if(ptr == NULL) throw(GetLastError());
		else return ptr;
	}
	void FreeMemInForeignProcess(HANDLE process, void* ptr) throw(...)
	{
		if(VirtualFreeEx(process, ptr, 0, MEM_RELEASE) == 0) throw(GetLastError());
	}
	void ReadFromForeignProcessMemory(HANDLE process, void* ptr, void* target, unsigned long size) throw(...)
	{
		if(ReadProcessMemory(process, ptr, target, size, NULL) == 0) throw(GetLastError());
	}
	void WriteToForeignProcessMemory(HANDLE process, void* ptr, void* src, unsigned long size) throw(...)
	{
		if(WriteProcessMemory(process, ptr, src, size, NULL) == 0) throw(GetLastError());
	}
	HANDLE FindExlorerProcess(HWND slave_wnd) throw(...)
	{ 
		// Thanks to mr_williams@rave.ch who pointed me to GetWindowThreadProcessId(), that makes this function waaaaaaaaaaaaay shorter.
		HANDLE proc;
		DWORD explorer_pid;

		// Get the PID based on a HWND. This is the good stuff. You wouldn't believe the long and difficult function I had to write before I heard of this simple API call.
		GetWindowThreadProcessId(slave_wnd, &explorer_pid);
		// Get a process handle which we need for the shared memory functions.
		proc = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION, FALSE, explorer_pid);
		if(proc == NULL) throw(GetLastError());
		else return proc;
	}
	/*
	* Here we query the listview in of the desktop for the icons that are on the desktop. Since we're talking to a foreign process (explorer.exe), we need to use shared memory to get the data.
	*/
	void GetDesktopIcons(std::list<icon_t>& icons) throw(...)
	{
		HWND listview_wnd = 0;
		HANDLE explorer = 0;
		unsigned long iconcount = 0;
		icon_t* iconbuffer = 0;
		void* ipc_iconpos = 0;
		void* ipc_iconlabel = 0;
		TCHAR* ipc_buffer = 0;
		bool error = false;
		LRESULT msg_result = 0;
		POINT iconpos;
		LVITEM iconlabel;
		TCHAR buffer[sizeof(TCHAR)*(MAX_PATH+1)];

		try
		{
			// Get the HWND of the listview
			listview_wnd = FindListView();
			// Get the total number of icons on the desktop
			iconcount = static_cast<unsigned long>(SendMessage(listview_wnd, LVM_GETITEMCOUNT, 0, 0));
			// No icons? Very unlikely, but whatever!
			if(iconcount == 0) return;

			// Get the PID of the process that houses the listview, i.e.: Explorer.exe
			explorer = FindExlorerProcess(listview_wnd);
			// Here we allocate the shared memory buffers to use in our little IPC.
			ipc_iconpos = AllocMemInForeignProcess(explorer, sizeof(POINT));
			ipc_iconlabel = AllocMemInForeignProcess(explorer, sizeof(LVITEM));
			ipc_buffer = static_cast<TCHAR*>(AllocMemInForeignProcess(explorer, sizeof(sizeof(TCHAR)*(MAX_PATH+1))));
			iconbuffer = new icon_t;

			// A big loop that will retrieve the data for all the icons on the desktop.
			for(unsigned long loop = 0; loop < iconcount; ++loop)
			{
				// First we get the icon position.
				msg_result = SendMessage(listview_wnd, LVM_GETITEMPOSITION, loop, reinterpret_cast<LPARAM>(ipc_iconpos));
				if(msg_result != TRUE) 
				{
					// Here the bad design and my laziness bites me. I use Windows error codes for exceptions, but a failure in this case won't give us a Windows error code, so I can't really throw anything and so I just show a messagebox.
					// If I ever update this program, I'll fix this, but as it stands, this program suits my needs.
					MessageBox(NULL, _T("Unable to get the icon position."), _T("Unable to obtain icon positions"), MB_OK|MB_ICONERROR);
					error = true;
					break;
				}

				// Get the data from the shared memory
				ReadFromForeignProcessMemory(explorer, ipc_iconpos, &iconpos, sizeof(POINT));

				// Set stuff up to retrieve the label of the icon.
				iconlabel.iSubItem = 0;
				iconlabel.cchTextMax = MAX_PATH;
				iconlabel.mask = LVIF_TEXT;
				iconlabel.pszText = (LPTSTR)ipc_buffer;

				WriteToForeignProcessMemory(explorer, ipc_iconlabel, &iconlabel, sizeof(LVITEM));

				// Request the label.
				msg_result = SendMessage(listview_wnd, LVM_GETITEMTEXT, loop, (LPARAM)ipc_iconlabel);
				if(msg_result < 0)
				{
					// Bad design again....don't try this at home kids.
					MessageBox(NULL, _T("Unable to get the icon label."), _T("Unable to obtain icon positions"), MB_OK|MB_ICONERROR);
					error = true;
					break;		
				}

				ReadFromForeignProcessMemory(explorer, ipc_buffer, &buffer, sizeof(buffer));

				iconbuffer->x = iconpos.x;
				iconbuffer->y = iconpos.y;
				iconbuffer->name = buffer;
				iconbuffer->index = loop;

				// Here we have all we need (coordinates and the label), so we stuff it in out list.
				icons.push_back(*iconbuffer);
			}

			// If an error occured, we better clear all the data we gathered.
			if(error) icons.clear();

			// Always clear up afterwards.
			FreeMemInForeignProcess(explorer, ipc_iconpos);
			FreeMemInForeignProcess(explorer, ipc_iconlabel);
			FreeMemInForeignProcess(explorer, ipc_buffer);
			CloseHandle(explorer);
		}
		catch(DWORD error)
		{	
			// This extra try..catch thing is to make sure I release all the stuff I allocated before the app quits.
			delete iconbuffer;
			if(explorer)
			{
				if(ipc_iconpos) FreeMemInForeignProcess(explorer, ipc_iconpos);
				if(ipc_iconlabel) FreeMemInForeignProcess(explorer, ipc_iconlabel);
				if(ipc_buffer) FreeMemInForeignProcess(explorer, ipc_buffer);
				CloseHandle(explorer);
			}

			// Rethrow the original error.
			throw error;
		}
	}
	// end of: following code is from http://www.codeguru.com/cpp/misc/misc/article.php/c3807/Obtaining-Icon-Positions.htm

	bool clickedOnDesktopIcon ()
	{
// 		HWND  hwndSysListView32 = getDesktopHandleBruteForce();
// 		if (hwndSysListView32 != 0)
// 		{
// 			int const count = ListView_GetItemCount(hwndSysListView32);
// 			for (int i = 0; i < count; ++i)
// 			{
// 				RECT rc = { };
// 				BOOL res = ListView_GetItemRect(hwndSysListView32, i, &rc, LVIR_BOUNDS);
// 				if (res == TRUE)
// 					return true;
// 			}
// 		}
		return false;
	}

	void destroyMenuIfInMargin (Gfx * gfx, bb::GuiWidget * w, POINT p, int margin)
	{
		RECT r;
		::GetWindowRect(w->m_gfxWindow->m_hwnd, &r);
		r.left += margin;
		r.top += margin;
		r.right -= margin;
		r.bottom -= margin;
		if (::PtInRect(&r, p))
		{
			gfx->DestroyWindow(w->GetId().c_str());
			w = nullptr;
		}
	};

	void BlackBox::ToggleDesktopMenu (bbstring const & widget_name)
	{
		POINT p;
		if (::GetCursorPos(&p))
		{
			GuiWidget * w = m_gfx->FindWidget(widget_name.c_str());
			HWND const clicked_window = ::WindowFromPoint(p);
			if (isDesktopHandle(clicked_window))
			{
				if (clickedOnDesktopIcon())
				{
					return;
				}
				if (!w)
				{
					w = m_gfx->MkWidgetFromId(widget_name.c_str());
				}
				else
				{
					destroyMenuIfInMargin(m_gfx.get(), w, p, -5);
				}
			}
			else if (w && clicked_window == w->m_gfxWindow->m_hwnd)
			{
				destroyMenuIfInMargin(m_gfx.get(), w, p, 5);
			}
			else if (w && clicked_window != w->m_gfxWindow->m_hwnd)
			{
				destroyMenuIfInMargin(m_gfx.get(), w, p, -5);
			}

			if (w)
			{
				w->MoveWindow(p.x, p.y);
				m_tasks.Focus(w->m_gfxWindow->m_hwnd);
				setOnTop(w->m_gfxWindow->m_hwnd);
				w->Show(true);
			}
		}
	}

	void BlackBox::ToggleMenu (std::shared_ptr<bb::MenuConfig> menu_cfg)
	{
		bool new_menu = false;
		GuiWidget * w = m_gfx->FindWidget(menu_cfg->m_id.c_str());
		if (!w)
		{
			w = m_gfx->MkWidgetFromConfig(*menu_cfg);
			new_menu = true;
		}

		POINT p;
		if (::GetCursorPos(&p))
		{
			GuiWidget * w = m_gfx->FindWidget(menu_cfg->m_id.c_str());
			HWND const clicked_window = ::WindowFromPoint(p);
			if (!w)
			{
				w = m_gfx->MkWidgetFromConfig(*menu_cfg);
			}
			else if (w && clicked_window == w->m_gfxWindow->m_hwnd)
			{
				destroyMenuIfInMargin(m_gfx.get(), w, p, 5);
			}
			else if (w && clicked_window != w->m_gfxWindow->m_hwnd)
			{
				destroyMenuIfInMargin(m_gfx.get(), w, p, -5);
			}

			if (w)
			{
				w->MoveWindow(p.x, p.y);
				m_tasks.Focus(w->m_gfxWindow->m_hwnd);
				setOnTop(w->m_gfxWindow->m_hwnd);
				w->Show(true);
			}
		}
	}

	void BlackBox::MakeSticky (HWND hwnd)
	{
	}

	void BlackBox::RemoveSticky (HWND hwnd)
	{
	}

	HWND BlackBox::GetHwnd ()
	{
		return m_hwnd;
	}

	void * BlackBox::GetSettingPtr (int sn_index)
	{
		if (m_style)
		{
			void * const ptr = m_style->GetStyleMemberPtr(sn_index);
			return ptr;
		}
		else
		{
			return m_defaultStyle->GetStyleMemberPtr(sn_index);
		}
	}

	bool BlackBox::GetConfigDir (wchar_t * dir, size_t dir_sz) const
	{
// 		wchar_t name[MAX_PATH];
// 		if (!m_cmdLine.ConfigDir().empty())
// 		{
// 			codecvt_utf8_utf16(m_cmdLine.ConfigDir(), dir, dir_sz);
// 			return true;
// 		}
// 		else if (getExeName(dir, MAX_PATH))
// 		{
// 			return true;
// 		}
		return false;
	}

	bool BlackBox::WorkSpacesSetCurrentVertexId (bbstring const & vertex_id)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE | CTX_BIND, "WorkSpacesSetCurrentVertexId: %ws", vertex_id.c_str());
		if (bbstring const * curr_vtx_id = m_wspaces.GetCurrentVertexId())
		{
			bbstring const curr_ws = *curr_vtx_id;
			if (m_wspaces.CanSetCurrentVertexId(vertex_id))
			{
				m_wspaces.SetCurrentVertexId(vertex_id);
				m_tasks.SwitchWorkSpace(curr_ws, vertex_id, false);
			}
		}
		return false;
	}

	bool BlackBox::WorkSpacesSwitchVertexViaEdge (bbstring const & edge_property)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE | CTX_BIND, "WorkSpacesSwitchVertexViaEdge edge=%ws", edge_property.c_str());
		bbstring new_vertex_id;
		if (FindTargetVertexViaEdge(edge_property, new_vertex_id))
			if (bbstring const * curr_vtx_id = m_wspaces.GetCurrentVertexId())
			{
				bbstring const curr_ws = *curr_vtx_id;
				m_wspaces.SetCurrentVertexId(new_vertex_id);
				m_tasks.SwitchWorkSpace(curr_ws, new_vertex_id, false);
				return true;
			}
		return false;
	}

	void BlackBox::MaximizeTopWindow (bool vertical)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE | CTX_BIND, "MaximizeTopWindow vertical=%i", vertical);
		if (HWND hwnd = FindTopLevelWindow())
			maximizeWindow(hwnd, vertical);
	}

	void BlackBox::SetTaskManIgnored (bbstring const & op)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE | CTX_BIND, "SetTaskManIgnored op=%ws", op.c_str());
		if (HWND hwnd = FindTopLevelWindow())
		{
			if (op == L"true")
				m_tasks.SetTaskManIgnored(hwnd);
			else if (op == L"false")
				m_tasks.UnsetTaskManIgnored(hwnd);
			else
			{
				m_tasks.ToggleTaskManIgnored(hwnd);
			}
		}
	}

	bool BlackBox::FindTargetVertexViaEdge (bbstring const & edge_property, bbstring & dst_vertex_id) const
	{
		if (m_wspaces.CanSwitchVertexViaEdge(edge_property, dst_vertex_id))
			return true;
		return false;
	}
	bool BlackBox::MoveWindowToVertex (HWND hwnd, bbstring const & vertex_id)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE | CTX_BIND, "MoveWindowToVertex edge=%ws", vertex_id.c_str());
		if (bbstring const * curr_vtx_id = m_wspaces.GetCurrentVertexId())
		{
			return m_tasks.MoveWindowToVertex(hwnd, vertex_id);
		}
		return false;
	}
	bool BlackBox::MoveTopWindowToVertexViaEdge (bbstring const & edge_property)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE | CTX_BIND, "MoveTopWindowToVertexViaEdge edge=%ws", edge_property.c_str());
		bbstring new_vertex_id;
		if (HWND hwnd = FindTopLevelWindow())
			if (FindTargetVertexViaEdge(edge_property, new_vertex_id))
				return m_tasks.MoveWindowToVertex(hwnd, new_vertex_id);
		return false;
	}

}

