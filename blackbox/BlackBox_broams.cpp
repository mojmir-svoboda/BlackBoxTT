	/* ==========================================================================

	This file is part of the bbLean source code
	Copyright � 2001-2003 The Blackbox for Windows Development Team
	Copyright � 2004-2009 grischka
	Copyright � 2015-2017 mojmir

	http://bb4win.sourceforge.net/bblean
	http://developer.berlios.de/projects/bblean

	bbLean is free software, released under the GNU General Public License
	(GPL version 2). For details see:

	http://www.fsf.org/licenses/gpl.html

	This program is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
	or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
	for more details.

	========================================================================== */
#include "BlackBox.h"
#include <blackbox/common.h>
#include <bblib/utils_paths.h>
#include "utils_window.h"
#include "cmd/Commands.h"
#include <bblibcompat/bblibcompat.h>

namespace bb {

	bool BlackBox::HandleBroamMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return m_broamServer.HandleWndMessage(hwnd, uMsg, wParam, lParam);
	}

	int exec_cfg_command (const wchar_t * argument);

	bool BroamServer::RegisterBroamListener (HWND handle, uint32_t const * msgs)
	{
		m_listeners.push_back(handle);
		return true;
	}

	bool BroamServer::UnregisterBroamListener (HWND handle, uint32_t const * msgs)
	{
		m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), handle), m_listeners.end());
		return true;
	}

	bool BroamServer::BroadcastBBMessage (UINT msgType, WPARAM wParam, LPARAM lParam)
	{
		if (msgType == BB_BROADCAST)
		{
			for (HWND listener : m_listeners)
				::SendMessage(listener, msgType, wParam, lParam);
		}
		else
		{
		}
		return true;
	}

	bool BroamServer::HandleWndMessage (HWND hwnd, UINT msgType, WPARAM wParam, LPARAM lParam)
	{
		wchar_t const * msg = reinterpret_cast<wchar_t const *>(lParam);
		switch (msgType)
		{
			case BB_REGISTERMESSAGE:
			{
				UINT const * const messages = reinterpret_cast<UINT const *>(lParam);
				HWND handle = (HWND)wParam;
				RegisterBroamListener(handle, messages);
				return true;
			}
			case BB_UNREGISTERMESSAGE:
			{
				UINT const * const messages = reinterpret_cast<UINT const *>(lParam);
				HWND handle = (HWND)wParam;
				RegisterBroamListener(handle, messages);
				return true;
			}
			case BB_POSTSTRING:
			{
				// posted command-string, from menu click
				exec_command(msg);
				delete[] msg; // @NOTE: delete is mandatory
				return true;
			}
			case BB_BROADCAST:
			{
				if (false == exec_broam(msg))
					BroadcastBBMessage(msgType, wParam, lParam);
				return true;
			}
		}

		if (IsBroam(msgType))
			return HandleCoreBroam(hwnd, msgType, wParam, msg);
		return false;
	}

	bool BroamServer::HandleCoreBroam (HWND hwnd, uint32_t uMsg, WPARAM wParam, wchar_t const * msg)
	{
#ifdef LOG_BB_MESSAGES
		//dbg_printf("hwnd %04x msg %d wp %x lp %x", hwnd, uMsg, wParam, lParam);
		if (uMsg >= BB_MSGFIRST && uMsg < BB_MSGLAST)
			log_BBMessage(uMsg, wParam, lParam, g_stack_top - (DWORD_PTR)&hwnd);
#endif

		switch (uMsg)
		{
			// first the BB_... internal messages
			//===================================
		case BB_QUIT:

			//MessageManager_Send(BB_EXITTYPE, 0, B_QUIT);
			/* clean up */
			//shutdown_blackbox();
			//PostQuitMessage(0);
			return true;

		case BB_SHUTDOWN:
			return true;

		case BB_SETSTYLE:
			return true;

		case BB_ABOUTPLUGINS:
			return true;

		case BB_ABOUTSTYLE:
			return true;

		case BB_EDITFILE:
			return true;

		case BB_RUN:
//			show_run_dlg();
			return true;

			//====================
			// Execute a string (shellcommand or broam)
		case BB_EXECUTE:
			exec_command(msg);
			return true;

		case BB_EXECUTEASYNC:
			PostCommand(msg);
			return true;

			//====================
		case BB_TOGGLEPLUGINS:
			//SendMessage(hwnd, BB_BROADCAST, 0, (LPARAM) ((wParam ? (int)wParam > 0 : g_PluginsHidden) ? "@BBShowPlugins" : "@BBHidePlugins"));
			goto dispatch_bb_message;

			//====================
		case BB_DESKCLICK:
			return true;

			//====================
			// Menu
		case BB_MENU:
			//if (MenuMaker_ShowMenu(wParam, (wchar_t const *)lParam))
			//	goto dispatch_bb_message;
			return true;

		case BB_HIDEMENU:
			//Menu_All_Hide();
			//goto dispatch_bb_message;

			//======================================================
		case BB_SETTHEME:
			//str = (wchar_t const *)lParam;
			//goto do_restart;

		case BB_RESTART:
// 		case_bb_restart:
// 			str = NULL;
// 		do_restart:
			// we dont want plugins being loaded twice.
// 			if (g_in_restart)
// 				break;
// 
// 			g_in_restart = true;
// 			MessageManager_Send(BB_EXITTYPE, 0, B_RESTART);
// 			kill_plugins();
// 			BBSleep(100);
// 
// 			if (lParam == BBOPT_PAUSE || (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
// 				BBMessageBox(MB_OK, NLS2("$Restart_Paused$",
// 					"Restart paused, press OK to continue..."));
// 			}
// 
// 			if (str) {
// 				writeString(getExtensionsRcPath(), TEXT("blackbox.theme:"), str);
// 				m_free((char*)str);
// 			}
// 
// 			register_fonts();
// 			Settings_menu.showBroams = false;
// 			Menu_All_Toggle(g_PluginsHidden = false);
// 			reconfigure_blackbox();
// 			MessageManager_Send(BB_RECONFIGURE, 0, 0);
// 			start_plugins();
// 			Menu_Update(MENU_UPD_ROOT);
// 
// 			BBSleep(100);
// 			g_in_restart = false;
			return true;

			//======================================================
		case BB_RECONFIGURE:
			goto dispatch_bb_message;

			//====================

		case BB_REDRAWGUI:
			goto dispatch_bb_message;

			//======================================================
			// forward these to Workspace.cpp

		case BB_WINDOWLOWER:
		case BB_WINDOWRAISE:
		case BB_WINDOWSHADE:
		case BB_WINDOWGROWHEIGHT:
		case BB_WINDOWGROWWIDTH:
// 			if (0 == msg && ::IsWindow((HWND)wParam))
// 				msg = wParam;

		case BB_WORKSPACE:
		case BB_SWITCHTON:
		case BB_LISTDESKTOPS:
		case BB_SENDWINDOWTON:
		case BB_MOVEWINDOWTON:
		case BB_BRINGTOFRONT:
		case BB_WINDOWMINIMIZE:
		case BB_WINDOWMAXIMIZE:
		case BB_WINDOWRESTORE:
		case BB_WINDOWCLOSE:
		case BB_WINDOWMOVE:
		case BB_WINDOWSIZE:
// 			LRESULT r = getWorkspaces().Command(uMsg, wParam, lParam);
// 			if (r != -1)
// 				return r; // ??????
			goto dispatch_bb_message;

			//====================
			// Updating of the workspaces/task menu is delayed in order
			// to get the correct window states with active or iconized.
		case BB_DESKTOPINFO:
		case BB_TASKSUPDATE:
			SetTimer(hwnd, BB_TASKUPDATE_TIMER, 200, NULL);
			goto dispatch_bb_message;

			//==============================================================
			// COPYDATA stuff, for passing information from/to other processes
			// (i.e. bbStyleMaker, BBNote)

		case BB_GETSTYLE:
//			return BBSendData((HWND)lParam, BB_SENDDATA, wParam, getStylePath().c_str(), -1);
			return true;

		case BB_GETSTYLESTRUCT:
//			return BBSendData((HWND)lParam, BB_SENDDATA, wParam, &mStyle, STYLESTRUCTSIZE);
			return true;

			// done with BB_messages,
			//==============================================================

			//====================
	dispatch_bb_message:
//			return MessageManager_Send(uMsg, wParam, lParam);
			return true;
		}
		return false;
	}

	/* execute a command, wait until execution finished (unless it's a shell command) */
	void BroamServer::exec_command(TCHAR const * cmd)
	{
		//TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Executing command: %s", cmd);
		if (NULL == cmd || 0 == cmd[0])
			return;
		if ('@' == cmd[0])
			SendMessage(m_BBHwnd, BB_BROADCAST, 0, (LPARAM)cmd);
		else
			Assert(0);
//			BBExecute_string(cmd, RUN_SHOWERRORS);
	}
	/* execute a command, wait until execution finished (unless it's a shell command) */
	void BroamServer::execCommand (tstring const & cmd)
	{
		exec_command(cmd.c_str());
	}

	/* post a formatted command, dont wait for execution but return immediately */
	bool BroamServer::PostCommand (wchar_t const * fmt, ...)
	{
		va_list arg_list;
		va_start(arg_list, fmt);
		//TRACE_MSG_VA(trace::e_Info, trace::CTX_BBCore, "Post command: %s", );
		wchar_t buff[e_broamMsgLenMax];
		size_t n = vswprintf(buff, e_broamMsgLenMax, fmt, arg_list);
		if (n > 0 && n < e_broamMsgLenMax)
		{
			wchar_t * msg = new wchar_t[n + 1];
			wcslcpy(msg, buff, n + 1);
			PostMessage(m_BBHwnd, BB_POSTSTRING, 0, (LPARAM)msg);
			return true;
		}
		return false;
	}

	void BroamServer::PostCommand (bbstring const & cmd)
	{
		PostCommand(L"%s", cmd.c_str());
	}

	/* check and parse "Workspace1" etc. strings */
	int get_workspace_number (const char * s)
	{
		if (0 == _memicmp(s, "workspace", 9))
		{
			int n = atoi(s + 9);
			if (n >= 1 && n <= 9)
				return n - 1;
		}
		return -1;
	}

	//===========================================================================
	enum E_ShutdownModes {
		BBSD_SHUTDOWN	= 0,
		BBSD_REBOOT		= 1,
		BBSD_LOGOFF		= 2,
		BBSD_HIBERNATE	= 3,
		BBSD_SUSPEND	= 4,
		BBSD_LOCKWS		= 5,
		BBSD_SWITCHUSER = 6,
		BBSD_EXITWIN	= 7,
	};

	enum E_CoreBroamCases {
		e_false = 0,
		e_true = 1,

		e_checkworkspace,
		e_rootCommand,
		e_Message,
		e_ShowAppnames,
		e_About,
		e_Nop,
		e_Pause,
		e_Crash,
		e_ShowRecoverMenu,
		e_RecoverWindow,
		e_Test,

		e_quiet,
		e_pause,
		e_bool,
	};

	enum E_CoreBroamFlags {
		e_mask	 = 0x01F,
		e_post	 = 0x020,
		e_lpstr  = 0x040,
		e_lpnum  = 0x080,
		e_lptask = 0x100,
		e_wpnum  = 0x200,
		e_lpint  = 0x400,
	};

	struct CoreBroamItem
	{
		const wchar_t * m_str;
		unsigned short m_msg;
		unsigned short m_flag;
		short m_wParam;
	};
	static const CoreBroamItem g_CoreBroams[] = {
		// one specific window
		{ L"Raise",					BB_WINDOWRAISE,		e_lptask, 0 },
		{ L"Lower",					BB_WINDOWLOWER,		e_lptask, 0 },
		{ L"Close",					BB_WINDOWCLOSE,		e_lptask, 0 },
		{ L"Minimize",				BB_WINDOWMINIMIZE,	e_lptask, 0 },
		{ L"Maximize",				BB_WINDOWMAXIMIZE,	e_lptask, 0 },
		{ L"Restore",				BB_WINDOWRESTORE,	e_lptask, 0 },
		{ L"Resize",					BB_WINDOWSIZE,		e_lptask, 0 },
		{ L"Move",					BB_WINDOWMOVE,		e_lptask, 0 },
		{ L"Shade",					BB_WINDOWSHADE,		e_lptask, 0 },
		// duplicates of the above -->
		{ L"RaiseWindow",			BB_WINDOWRAISE,		e_lptask, 0 },
		{ L"LowerWindow",			BB_WINDOWLOWER,		e_lptask, 0 },
		{ L"CloseWindow",			BB_WINDOWCLOSE,		e_lptask, 0 },
		{ L"MinimizeWindow",			BB_WINDOWMINIMIZE,	e_lptask, 0 },
		{ L"MaximizeWindow",			BB_WINDOWMAXIMIZE,	e_lptask, 0 },
		{ L"RestoreWindow",			BB_WINDOWRESTORE,	e_lptask, 0 },
		{ L"ResizeWindow",			BB_WINDOWSIZE,		e_lptask, 0 },
		{ L"MoveWindow",				BB_WINDOWMOVE,		e_lptask, 0 },
		{ L"ShadeWindow",			BB_WINDOWSHADE,		e_lptask, 0 },
		// <--

		{ L"MaximizeVertical",		BB_WINDOWGROWHEIGHT,e_lptask, 0 },
		{ L"MaximizeHorizontal",		BB_WINDOWGROWWIDTH, e_lptask, 0 },
		{ L"ActivateWindow",			BB_BRINGTOFRONT,	e_lptask, 0},

		{ L"StickWindow",			BB_WORKSPACE,		e_lptask, BBWS_TOGGLESTICKY },
		{ L"OnTopWindow",			BB_WORKSPACE,		e_lptask, BBWS_TOGGLEONTOP },
		{ L"OnBackgroundWindow",		BB_WORKSPACE,		e_lptask, BBWS_TOGGLEONBG },
		{ L"MoveWindowLeft",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWLEFT },
		{ L"MoveWindowRight",		BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWRIGHT },
		{ L"MoveWindowUp",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWUP },
		{ L"MoveWindowDown",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWDOWN },
		{ L"MoveWindowToWS",			BB_MOVEWINDOWTON,	e_lptask|e_wpnum, 0},
		{ L"SendWindowToWS",			BB_SENDWINDOWTON,	e_lptask|e_wpnum, 0},

		// cycle windows
		{ L"PrevWindow",				BB_WORKSPACE,		0, BBWS_PREVWINDOW },
		{ L"NextWindow",				BB_WORKSPACE,		0, BBWS_NEXTWINDOW },
		{ L"PrevWindowAllWorkspaces",BB_WORKSPACE,		e_true, BBWS_PREVWINDOW },
		{ L"NextWindowAllWorkspaces",BB_WORKSPACE,		e_true, BBWS_NEXTWINDOW },

		// all windows
		{ L"MinimizeAll",			BB_WORKSPACE,		0, BBWS_MINIMIZEALL },
		{ L"RestoreAll",				BB_WORKSPACE,		0, BBWS_RESTOREALL },
		{ L"Cascade",				BB_WORKSPACE,		0, BBWS_CASCADE },
		{ L"TileVertical",			BB_WORKSPACE,		0, BBWS_TILEVERTICAL },
		{ L"TileHorizontal",			BB_WORKSPACE,		0, BBWS_TILEHORIZONTAL },

		// workspaces
		{ L"LeftWorkspace",			BB_WORKSPACE,		0, BBWS_DESKLEFT },
		{ L"PrevWorkspace",			BB_WORKSPACE,		0, BBWS_DESKLEFT },
		{ L"UpWorkspace",			BB_WORKSPACE,		0, BBWS_DESKUP },
		{ L"DownWorkspace",			BB_WORKSPACE,		0, BBWS_DESKDOWN },
		{ L"RightWorkspace",			BB_WORKSPACE,		0, BBWS_DESKRIGHT },
		{ L"NextWorkspace",			BB_WORKSPACE,		0, BBWS_DESKRIGHT },
		{ L"LastWorkspace",			BB_WORKSPACE,		0, BBWS_LASTDESK },
		{ L"SwitchToWorkspace",		BB_WORKSPACE,		e_lpnum,  BBWS_SWITCHTODESK },

		{ L"Gather",					BB_WORKSPACE,		0, BBWS_GATHERWINDOWS },
		{ L"GatherWindows",			BB_WORKSPACE,		0, BBWS_GATHERWINDOWS },
		{ L"AddWorkspace",			BB_WORKSPACE,		0, BBWS_ADDDESKTOP },
		{ L"DelWorkspace",			BB_WORKSPACE,		0, BBWS_DELDESKTOP },
		{ L"EditWorkspaceNames",		BB_WORKSPACE,		e_post, BBWS_EDITNAME },
		{ L"SetWorkspaceNames",		BB_WORKSPACE,		e_lpstr, BBWS_EDITNAME },

		// menu
		{ L"ShowMenu",				BB_MENU,			e_lpstr,  BB_MENU_BROAM },
		{ L"ShowWorkspaceMenu",		BB_MENU,			0, BB_MENU_TASKS },
		{ L"ShowIconMenu",			BB_MENU,			0, BB_MENU_ICONS },
		{ L"HideMenu",				BB_HIDEMENU,		0, 0 },

		// blackbox
		{ L"TogglePlugins",			BB_TOGGLEPLUGINS,	e_bool, 0 },
		{ L"ToggleTray",				BB_TOGGLETRAY,		0, 0 },
		{ L"AboutStyle",				BB_ABOUTSTYLE,		e_post, 0 },
		{ L"AboutPlugins",			BB_ABOUTPLUGINS,	e_post, 0 },
		{ L"Reconfig",				BB_RECONFIGURE,		e_post, 0 },
		{ L"Reconfigure",			BB_RECONFIGURE,		e_post, 0 },
		{ L"Restart",				BB_RESTART,			e_post|e_pause, 0 },
		{ L"Exit",					BB_QUIT,			e_post|e_quiet, 0 },
		{ L"Quit",					BB_QUIT,			e_post|e_quiet, 0 },
		{ L"Run",					BB_RUN,				e_post, 0 },
		{ L"Theme",					BB_SETTHEME,		e_post|e_lpstr, 0 },

		// edit
		{ L"Edit",					BB_EDITFILE,		e_lpstr, -1},
		{ L"EditStyle",				BB_EDITFILE,		0, 0 },
		{ L"EditMenu",				BB_EDITFILE,		0, 1 },
		{ L"EditPlugins",			BB_EDITFILE,		0, 2 },
		{ L"EditExtensions",			BB_EDITFILE,		0, 3 },
		{ L"EditBlackbox",			BB_EDITFILE,		0, 4 },

		// shutdown
		{ L"Shutdown",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_SHUTDOWN	},
		{ L"Reboot",					BB_SHUTDOWN,		e_post|e_quiet, BBSD_REBOOT		},
		{ L"Logoff",					BB_SHUTDOWN,		e_post|e_quiet, BBSD_LOGOFF		},
		{ L"Hibernate",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_HIBERNATE	},
		{ L"Suspend",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_SUSPEND	},
		{ L"LockWorkstation",		BB_SHUTDOWN,		e_post|e_quiet, BBSD_LOCKWS		},
		{ L"SwitchUser",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_SWITCHUSER },
		{ L"ExitWindows",			BB_SHUTDOWN,		e_post|e_quiet, BBSD_EXITWIN	},

		// miscellaneous
		{ L"Style",					BB_SETSTYLE,		e_lpstr, 0 },
		{ L"Exec",					BB_EXECUTE,			e_lpstr, 0 },
		{ L"Post",					BB_EXECUTEASYNC,	e_lpstr, 0 },
		{ L"Label",					BB_SETTOOLBARLABEL, e_lpstr, 0 },

		{ L"rootCommand",			0, e_rootCommand	, 0 },
		{ L"Message",				0, e_Message		, 0 },
		{ L"ShowAppnames",			0, e_ShowAppnames	, 0 },
		{ L"ShowRecoverMenu",		0, e_ShowRecoverMenu , 0 },
		{ L"RecoverWindow",			0, e_RecoverWindow	, 0 },
		{ L"About",					0, e_About			, 0 },
		{ L"Pause",					0, e_Pause			, 0 },
		{ L"Nop",					0, e_Nop			, 0 },
		{ L"Crash",					0, e_Crash			, 0 },
		{ L"Test",					0, e_Test			, 0 },

		{ NULL /*"Workspace#"*/,	BB_WORKSPACE, e_checkworkspace,  BBWS_SWITCHTODESK },
	};

	int get_false_true(const wchar_t *arg)
	{
		if (arg) {
			if (0==_wcsicmp(arg, L"true"))
				return 1;
			if (0==_wcsicmp(arg, L"false"))
				return 0;
		}
		return -1;
	}

	bool BroamServer::IsBroam (uint32_t uMsg) const
	{
		for (CoreBroamItem const & item : g_CoreBroams)
		{
			if (item.m_msg == uMsg)
			{
				return true;
			}
		}
		return false;
	}

	//===========================================================================
	int BroamServer::exec_core_broam (TCHAR const * broam)
	{
		return false;
		//TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);

// 		TCHAR buffer[MAX_PATH];
// 		TCHAR num[MAX_PATH];
// 		TCHAR const * core_args = broam + sizeof "@BBCore." - 1;
// 		TCHAR * core_cmd = NextToken(buffer, MAX_PATH, core_args, NULL);
// 
// 		corebroam_table const * action = g_corebroam_table;
// 		do
// 		{
// 			if (0 ==_tcsicmp(action->m_str, core_cmd))
// 				break;
// 		}
// 		while ((++action)->m_str);
// 
// 		WPARAM wParam = action->m_wParam;
// 		LPARAM lParam = 0;
// 		UINT msg = action->m_msg;
// 
// 		if (action->m_flag & e_wpnum)
// 			wParam |= atoi(NextToken(num, MAX_PATH, core_args, NULL)) - 1;
// 
// 		if (action->m_flag & e_lpstr)
// 		{
// 			bool const post = action->m_flag & e_post;
// 			wchar_t const * str = post ? wcsdup(core_args) : core_args;
// 			lParam = (LPARAM)(str);
// 		}
// 		else
// 			if (action->m_flag & e_lpnum)
// 				lParam = _wtoi(core_args)-1;
// 			else
// 				if (action->m_flag & e_lpint)
// 					lParam = _wtoi(core_args);
// 				else
// 					if (action->m_flag & e_lptask)
// 					{
// 						HWND hwnd = nullptr; //getWorkspaces().GetTask(_wtoi(core_args)-1);
// 						lParam = (LPARAM)hwnd;
// 					}
// 
// 		switch (action->m_flag & e_mask)
// 		{
// 			case e_checkworkspace:
// 			{
// 				Assert(0 && "not implemented");
// // 				int const n = get_workspace_number(core_cmd);
// // 				if (-1 == n)
// // 					return 0;
// // 				lParam = n;
// 				break;
// 			}
// 			case e_quiet:
// 				// check for 'no confirmation' option
// 				if (0 == _memicmp(core_args, "-q"/*uiet*/, 2))
// 					lParam = BBOPT_QUIET;
// 				break;
// 
// 			case e_pause:
// 				// check for 'pause restart' option
// 				if (0 == _memicmp(core_args, "-p"/*ause*/, 2))
// 					lParam = BBOPT_PAUSE;
// 				break;
// 
// 			case e_true: // ...AllWorkspaces option
// 				lParam = 1;
// 				break;
// 
// 			case e_bool:
// 				wParam = 1 + get_false_true(core_args);
// 				break;
// 
// 				// --- special (no message) commands ---
// 			case e_rootCommand:
// // 				Desk_new_background(core_args);
// // 				PostMessage(BBhwnd, BB_MENU, BB_MENU_UPDATE, 0);
// 				break;
// 
// 			case e_Message:
// 				BBMessageBox(MB_OK, L"%s", core_args);
// 				break;
// 
// 			case e_ShowAppnames:
// //				ShowAppnames();
// 				break;
// 
// 			case e_About:
// //				bb_about();
// 				break;
// 
// 			case e_Nop:
// 				break;
// 
// 			case e_Pause:
// 			{
// 				Assert(0 && "pause not implemented");
// //				BBSleep(atoi(core_args));
// 				break;
// 			}
// 			case e_Crash:
// 				*(DWORD*)0 = 0x11111111;
// 				break;
// 
// 			case e_ShowRecoverMenu:
// 			{
// 				Assert(0 && "show recover menu not implemented");
// 				//ShowRecoverMenu();
// 				break;
// 			}
// 			case e_RecoverWindow:
// 			{
// 				Assert(0 && "show recover win not implemented");
// // 				HWND hwnd;
// // 				if (sscanf(core_args, "%p", &hwnd))
// // 					getWorkspaces().ToggleWindowVisibility(hwnd);
// 				break;
// 			}
// 			case e_Test:
// 				break;
// 			}
// 
// 		if (msg)
// 		{
// 			// Some things need to be 'Post'ed, i.e. to return from plugins
// 			// before they are unloaded with restart, quit etc.
// 			if (action->m_flag & e_post)
// 				PostMessage(BBhwnd, msg, wParam, lParam);
// 			else
// 				SendMessage(BBhwnd, msg, wParam, lParam);
// 		}
//		return 1;
	}

// 	/* ----------------------------------------------------------------------- */
// 	/* This is for the menu checkmarks in the styles and backgrounds folders */
// 	bool get_opt_command (char * opt_cmd, const char * cmd)
// 	{
// 		//TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "cmd = %s", cmd);
// 		if (0 == opt_cmd[0])
// 		{
// 			if (0 == _memicmp(cmd, "@BBCore.", 8))
// 			{
// 				// internals, currently for style and rootcommand
// #define CHECK_BROAM(broam) 0 == _memicmp(cmd, s = broam, sizeof broam - 3)
// 				const char *s;
// 				if (CHECK_BROAM(MM_STYLE_BROAM))
// 					sprintf(opt_cmd, s, getStylePath().c_str());
// 				else if (CHECK_BROAM(MM_THEME_BROAM))
// 					sprintf(opt_cmd, s, g_defaultrc_path[0] ? g_defaultrc_path : "default");
// 				else if (CHECK_BROAM(MM_ROOT_BROAM))
// 					sprintf(opt_cmd, s, Desk_extended_rootCommand(NULL));
// 				else
// 					return false;
// #undef CHECK_BROAM
// 			}
// 			else if (0 == MessageManager_Send(BB_GETBOOL, (WPARAM)opt_cmd, (LPARAM)cmd))
// 				return false; // nothing found
// 			else if (opt_cmd[0] == 1)
// 			{
// 				opt_cmd[0] = 0; // recheck next time;
// 				return true;
// 			}
// 		}
// 		return opt_cmd[0] && 0 == _stricmp(opt_cmd, cmd);
// 	}

	//===========================================================================
	bool BroamServer::exec_script (const wchar_t * broam)
	{
		//TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);
// 		const TCHAR * p = 0, *a = 0;
// 		TCHAR * s = 0;
// 		int n = 0, c = 0;
// 		if ('[' != skip_spc(&broam))
// 			return false;
// 		for (n = _tcslen(++broam); n && (c = broam[--n], IS_SPC(c));)
// 			;
// 		if (0 == n || ']' != c)
// 			return false;
// 		s = new_str_n(broam, n);
// 		for (p = s; 0 != (n = nexttoken(a, p, TEXT("|")));)
// 		{
// 			s[a - s + n] = 0;
// 			exec_command(a);
// 		}
// 		free(s);
		return true;
	}

	//===========================================================================
	// returns 'true' for done, 'false' for pass on to plugins
	bool BroamServer::exec_broam (const wchar_t * broam)
	{
// 		//TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);
// 		if (0 == _memicmp(broam, "@BBCore.", 8))
// 		{
// 			if (0 == exec_core_broam(broam))
// 				goto broam_error;
// 		}
// 		else if (0 == _memicmp(broam, "@BBCfg.", 7))
// 		{
// 			if (0 == exec_cfg_command(broam+7))
// 				goto broam_error;
// 		}
// 		else if (0==_memicmp(broam, "@Script", 7))
// 		{
// 			exec_script(broam + 7);
// 			return true;
// 		}
// 		else if (0==_stricmp(broam, "@BBHidePlugins"))
// 		{
// 			Menu_All_Toggle(g_PluginsHidden = true);
// 		}
// 		else if (0==_stricmp(broam, "@BBShowPlugins"))
// 		{
// 			Menu_All_Toggle(g_PluginsHidden = false);
// 		}
// 		return false;
// 
// 	broam_error:
// 		BBMessageBox(MB_OK, NLS2("$Error_UnknownBroam$", "Error: Unknown Broadcast Message:\n%s"), broam);
// 		//TRACE_MSG(trace::e_Error, trace::CTX_BBCore, "Error: Unknown Broadcast Message: %s", broam);
		return false;
	}
}