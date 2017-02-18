	/* ==========================================================================

	This file is part of the bbLean source code
	Copyright © 2001-2003 The Blackbox for Windows Development Team
	Copyright © 2004-2009 grischka
	Copyright © 2015-2017 mojmir

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

namespace bb {

	LRESULT CALLBACK bbMainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT r;
		const char *str;
		const WPARAM ID_HOTKEY = 3;

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
			break;

		case BB_SHUTDOWN:
			break;

		case BB_SETSTYLE:
			break;

		case BB_ABOUTPLUGINS:
			break;

		case BB_ABOUTSTYLE:
			break;

		case BB_EDITFILE:
			break;

		case BB_RUN:
			show_run_dlg();
			break;

			//====================
			// Execute a string (shellcommand or broam)
		case BB_EXECUTE:
			//exec_command((const char*)lParam);
			break;

		case BB_POSTSTRING: // posted command-string, from menu click
												//exec_command((const char*)lParam);
												//m_free((char*)lParam);
			break;

		case BB_EXECUTEASYNC:
			//post_command((const char*)lParam);
			break;

			//====================
		case BB_BROADCAST:
			//if (false == exec_broam((const char*)lParam))
			//	goto dispatch_bb_message;
			break;

			//====================
		case BB_TOGGLEPLUGINS:
			//SendMessage(hwnd, BB_BROADCAST, 0, (LPARAM) ((wParam ? (int)wParam > 0 : g_PluginsHidden) ? "@BBShowPlugins" : "@BBHidePlugins"));
			goto dispatch_bb_message;

			//====================
		case BB_DESKCLICK:
			break;

			//====================
			// Menu
		case BB_MENU:
			//if (MenuMaker_ShowMenu(wParam, (const char*)lParam))
			//	goto dispatch_bb_message;
			break;

		case BB_HIDEMENU:
			//Menu_All_Hide();
			//goto dispatch_bb_message;

			//======================================================
		case BB_SETTHEME:
			//str = (const char*)lParam;
			//goto do_restart;

		case BB_RESTART:
		case_bb_restart:
			str = NULL;
		do_restart:
			// we dont want plugins being loaded twice.
			if (g_in_restart)
				break;

			g_in_restart = true;
			MessageManager_Send(BB_EXITTYPE, 0, B_RESTART);
			kill_plugins();
			BBSleep(100);

			if (lParam == BBOPT_PAUSE || (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
				BBMessageBox(MB_OK, NLS2("$Restart_Paused$",
					"Restart paused, press OK to continue..."));
			}

			if (str) {
				writeString(getExtensionsRcPath(), TEXT("blackbox.theme:"), str);
				m_free((char*)str);
			}

			register_fonts();
			Settings_menu.showBroams = false;
			Menu_All_Toggle(g_PluginsHidden = false);
			reconfigure_blackbox();
			MessageManager_Send(BB_RECONFIGURE, 0, 0);
			start_plugins();
			Menu_Update(MENU_UPD_ROOT);

			BBSleep(100);
			g_in_restart = false;
			break;

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
			if (0 == lParam && IsWindow((HWND)wParam))
				lParam = wParam;

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
			//r = getWorkspaces().Command(uMsg, wParam, lParam);
			if (r != -1) return r;
			goto dispatch_bb_message;

			//====================
			// Updating of the workspaces/task menu is delayed in order
			// to get the correct window states with active or iconized.
		case BB_DESKTOPINFO:
		case BB_TASKSUPDATE:
			SetTimer(hwnd, BB_TASKUPDATE_TIMER, 200, NULL);
			goto dispatch_bb_message;

			//====================
		case BB_REGISTERMESSAGE:
			MessageManager_Register((HWND)wParam, (UINT*)lParam, true);
			break;

		case BB_UNREGISTERMESSAGE:
			MessageManager_Register((HWND)wParam, (UINT*)lParam, false);
			break;

			//==============================================================
			// COPYDATA stuff, for passing information from/to other processes
			// (i.e. bbStyleMaker, BBNote)

		case BB_GETSTYLE:
			return BBSendData((HWND)lParam, BB_SENDDATA, wParam, getStylePath().c_str(), -1);

		case BB_GETSTYLESTRUCT:
			return BBSendData((HWND)lParam, BB_SENDDATA, wParam, &mStyle, STYLESTRUCTSIZE);

			// done with BB_messages,
			//==============================================================

			//====================
			//dispatch_bb_message:
			//return MessageManager_Send(uMsg, wParam, lParam);

			//====================
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	/* execute a command, wait until execution finished (unless it's a shell command) */
	void exec_command(TCHAR const * cmd)
	{
		TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Executing command: %s", cmd);
		if (NULL == cmd || 0 == cmd[0])
			return;
		if ('@' == cmd[0])
			SendMessage(BBhwnd, BB_BROADCAST, 0, (LPARAM)cmd);
		else
			BBExecute_string(cmd, RUN_SHOWERRORS);
	}
	/* execute a command, wait until execution finished (unless it's a shell command) */
	void execCommand(tstring const & cmd)
	{
		exec_command(cmd.c_str());
	}

	/* post a formatted command, dont wait for execution but return immediately */
	void post_command_fmt (const char * fmt, ...)
	{
		va_list arg_list;
		va_start(arg_list, fmt);
		//TRACE_MSG_VA(trace::e_Info, trace::CTX_BBCore, "Post command: %s", );
		PostMessage(BBhwnd, BB_POSTSTRING, 0, (LPARAM)m_formatv(fmt, arg_list));
	}

	/* post a command, dont wait for execution but return immediately */
	void post_command (const char * cmd)
	{
		post_command_fmt("%s", cmd);
	}
	void post_command (tstring const & cmd)
	{
		post_command_fmt("%s", cmd.c_str());
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
	enum shutdown_modes {
		BBSD_SHUTDOWN	= 0,
		BBSD_REBOOT		= 1,
		BBSD_LOGOFF		= 2,
		BBSD_HIBERNATE	= 3,
		BBSD_SUSPEND	= 4,
		BBSD_LOCKWS		= 5,
		BBSD_SWITCHUSER = 6,
		BBSD_EXITWIN	= 7,
	};

	enum corebroam_cases {
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

	enum corebroam_flags {
		e_mask	 = 0x01F,
		e_post	 = 0x020,
		e_lpstr  = 0x040,
		e_lpnum  = 0x080,
		e_lptask = 0x100,
		e_wpnum  = 0x200,
		e_lpint  = 0x400,
	};

	struct corebroam_table
	{
		const char * m_str;
		unsigned short m_msg;
		unsigned short m_flag;
		short m_wParam;
	};
	static const corebroam_table g_corebroam_table[] = {
		// one specific window
		{ "Raise",					BB_WINDOWRAISE,		e_lptask, 0 },
		{ "Lower",					BB_WINDOWLOWER,		e_lptask, 0 },
		{ "Close",					BB_WINDOWCLOSE,		e_lptask, 0 },
		{ "Minimize",				BB_WINDOWMINIMIZE,	e_lptask, 0 },
		{ "Maximize",				BB_WINDOWMAXIMIZE,	e_lptask, 0 },
		{ "Restore",				BB_WINDOWRESTORE,	e_lptask, 0 },
		{ "Resize",					BB_WINDOWSIZE,		e_lptask, 0 },
		{ "Move",					BB_WINDOWMOVE,		e_lptask, 0 },
		{ "Shade",					BB_WINDOWSHADE,		e_lptask, 0 },
		// duplicates of the above -->
		{ "RaiseWindow",			BB_WINDOWRAISE,		e_lptask, 0 },
		{ "LowerWindow",			BB_WINDOWLOWER,		e_lptask, 0 },
		{ "CloseWindow",			BB_WINDOWCLOSE,		e_lptask, 0 },
		{ "MinimizeWindow",			BB_WINDOWMINIMIZE,	e_lptask, 0 },
		{ "MaximizeWindow",			BB_WINDOWMAXIMIZE,	e_lptask, 0 },
		{ "RestoreWindow",			BB_WINDOWRESTORE,	e_lptask, 0 },
		{ "ResizeWindow",			BB_WINDOWSIZE,		e_lptask, 0 },
		{ "MoveWindow",				BB_WINDOWMOVE,		e_lptask, 0 },
		{ "ShadeWindow",			BB_WINDOWSHADE,		e_lptask, 0 },
		// <--

		{ "MaximizeVertical",		BB_WINDOWGROWHEIGHT,e_lptask, 0 },
		{ "MaximizeHorizontal",		BB_WINDOWGROWWIDTH, e_lptask, 0 },
		{ "ActivateWindow",			BB_BRINGTOFRONT,	e_lptask, 0},

		{ "StickWindow",			BB_WORKSPACE,		e_lptask, BBWS_TOGGLESTICKY },
		{ "OnTopWindow",			BB_WORKSPACE,		e_lptask, BBWS_TOGGLEONTOP },
		{ "OnBackgroundWindow",		BB_WORKSPACE,		e_lptask, BBWS_TOGGLEONBG },
		{ "MoveWindowLeft",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWLEFT },
		{ "MoveWindowRight",		BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWRIGHT },
		{ "MoveWindowUp",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWUP },
		{ "MoveWindowDown",			BB_WORKSPACE,		e_lptask, BBWS_MOVEWINDOWDOWN },
		{ "MoveWindowToWS",			BB_MOVEWINDOWTON,	e_lptask|e_wpnum, 0},
		{ "SendWindowToWS",			BB_SENDWINDOWTON,	e_lptask|e_wpnum, 0},

		// cycle windows
		{ "PrevWindow",				BB_WORKSPACE,		0, BBWS_PREVWINDOW },
		{ "NextWindow",				BB_WORKSPACE,		0, BBWS_NEXTWINDOW },
		{ "PrevWindowAllWorkspaces",BB_WORKSPACE,		e_true, BBWS_PREVWINDOW },
		{ "NextWindowAllWorkspaces",BB_WORKSPACE,		e_true, BBWS_NEXTWINDOW },

		// all windows
		{ "MinimizeAll",			BB_WORKSPACE,		0, BBWS_MINIMIZEALL },
		{ "RestoreAll",				BB_WORKSPACE,		0, BBWS_RESTOREALL },
		{ "Cascade",				BB_WORKSPACE,		0, BBWS_CASCADE },
		{ "TileVertical",			BB_WORKSPACE,		0, BBWS_TILEVERTICAL },
		{ "TileHorizontal",			BB_WORKSPACE,		0, BBWS_TILEHORIZONTAL },

		// workspaces
		{ "LeftWorkspace",			BB_WORKSPACE,		0, BBWS_DESKLEFT },
		{ "PrevWorkspace",			BB_WORKSPACE,		0, BBWS_DESKLEFT },
		{ "UpWorkspace",			BB_WORKSPACE,		0, BBWS_DESKUP },
		{ "DownWorkspace",			BB_WORKSPACE,		0, BBWS_DESKDOWN },
		{ "RightWorkspace",			BB_WORKSPACE,		0, BBWS_DESKRIGHT },
		{ "NextWorkspace",			BB_WORKSPACE,		0, BBWS_DESKRIGHT },
		{ "LastWorkspace",			BB_WORKSPACE,		0, BBWS_LASTDESK },
		{ "SwitchToWorkspace",		BB_WORKSPACE,		e_lpnum,  BBWS_SWITCHTODESK },

		{ "Gather",					BB_WORKSPACE,		0, BBWS_GATHERWINDOWS },
		{ "GatherWindows",			BB_WORKSPACE,		0, BBWS_GATHERWINDOWS },
		{ "AddWorkspace",			BB_WORKSPACE,		0, BBWS_ADDDESKTOP },
		{ "DelWorkspace",			BB_WORKSPACE,		0, BBWS_DELDESKTOP },
		{ "EditWorkspaceNames",		BB_WORKSPACE,		e_post, BBWS_EDITNAME },
		{ "SetWorkspaceNames",		BB_WORKSPACE,		e_lpstr, BBWS_EDITNAME },

		// menu
		{ "ShowMenu",				BB_MENU,			e_lpstr,  BB_MENU_BROAM },
		{ "ShowWorkspaceMenu",		BB_MENU,			0, BB_MENU_TASKS },
		{ "ShowIconMenu",			BB_MENU,			0, BB_MENU_ICONS },
		{ "HideMenu",				BB_HIDEMENU,		0, 0 },

		// blackbox
		{ "TogglePlugins",			BB_TOGGLEPLUGINS,	e_bool, 0 },
		{ "ToggleTray",				BB_TOGGLETRAY,		0, 0 },
		{ "AboutStyle",				BB_ABOUTSTYLE,		e_post, 0 },
		{ "AboutPlugins",			BB_ABOUTPLUGINS,	e_post, 0 },
		{ "Reconfig",				BB_RECONFIGURE,		e_post, 0 },
		{ "Reconfigure",			BB_RECONFIGURE,		e_post, 0 },
		{ "Restart",				BB_RESTART,			e_post|e_pause, 0 },
		{ "Exit",					BB_QUIT,			e_post|e_quiet, 0 },
		{ "Quit",					BB_QUIT,			e_post|e_quiet, 0 },
		{ "Run",					BB_RUN,				e_post, 0 },
		{ "Theme",					BB_SETTHEME,		e_post|e_lpstr, 0 },

		// edit
		{ "Edit",					BB_EDITFILE,		e_lpstr, -1},
		{ "EditStyle",				BB_EDITFILE,		0, 0 },
		{ "EditMenu",				BB_EDITFILE,		0, 1 },
		{ "EditPlugins",			BB_EDITFILE,		0, 2 },
		{ "EditExtensions",			BB_EDITFILE,		0, 3 },
		{ "EditBlackbox",			BB_EDITFILE,		0, 4 },

		// shutdown
		{ "Shutdown",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_SHUTDOWN	},
		{ "Reboot",					BB_SHUTDOWN,		e_post|e_quiet, BBSD_REBOOT		},
		{ "Logoff",					BB_SHUTDOWN,		e_post|e_quiet, BBSD_LOGOFF		},
		{ "Hibernate",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_HIBERNATE	},
		{ "Suspend",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_SUSPEND	},
		{ "LockWorkstation",		BB_SHUTDOWN,		e_post|e_quiet, BBSD_LOCKWS		},
		{ "SwitchUser",				BB_SHUTDOWN,		e_post|e_quiet, BBSD_SWITCHUSER },
		{ "ExitWindows",			BB_SHUTDOWN,		e_post|e_quiet, BBSD_EXITWIN	},

		// miscellaneous
		{ "Style",					BB_SETSTYLE,		e_lpstr, 0 },
		{ "Exec",					BB_EXECUTE,			e_lpstr, 0 },
		{ "Post",					BB_EXECUTEASYNC,	e_lpstr, 0 },
		{ "Label",					BB_SETTOOLBARLABEL, e_lpstr, 0 },

		{ "rootCommand",			0, e_rootCommand	, 0 },
		{ "Message",				0, e_Message		, 0 },
		{ "ShowAppnames",			0, e_ShowAppnames	, 0 },
		{ "ShowRecoverMenu",		0, e_ShowRecoverMenu , 0 },
		{ "RecoverWindow",			0, e_RecoverWindow	, 0 },
		{ "About",					0, e_About			, 0 },
		{ "Pause",					0, e_Pause			, 0 },
		{ "Nop",					0, e_Nop			, 0 },
		{ "Crash",					0, e_Crash			, 0 },
		{ "Test",					0, e_Test			, 0 },

		{ NULL /*"Workspace#"*/,	BB_WORKSPACE, e_checkworkspace,  BBWS_SWITCHTODESK },
	};

	//===========================================================================
	int exec_core_broam(TCHAR const * broam)
	{
		TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);

		TCHAR buffer[MAX_PATH];
		TCHAR num[MAX_PATH];
		TCHAR const * core_args = broam + sizeof "@BBCore." - 1;
		TCHAR * core_cmd = NextToken(buffer, MAX_PATH, core_args, NULL);

		corebroam_table const * action = g_corebroam_table;
		do
		{
			if (0 ==_tcsicmp(action->m_str, core_cmd))
				break;
		}
		while ((++action)->m_str);

		WPARAM wParam = action->m_wParam;
		LPARAM lParam = 0;
		UINT msg = action->m_msg;

		if (action->m_flag & e_wpnum)
			wParam |= atoi(NextToken(num, MAX_PATH, core_args, NULL)) - 1;

		if (action->m_flag & e_lpstr)
			lParam = (LPARAM)(action->m_flag & e_post ? new_str(core_args) : core_args);
		else
			if (action->m_flag & e_lpnum)
				lParam = atoi(core_args)-1;
			else
				if (action->m_flag & e_lpint)
					lParam = atoi(core_args);
				else
					if (action->m_flag & e_lptask)
						lParam = (LPARAM)getWorkspaces().GetTask(atoi(core_args)-1);

		switch (action->m_flag & e_mask)
		{
		case e_checkworkspace:
		{
			int const n = get_workspace_number(core_cmd);
			if (-1 == n)
				return 0;
			lParam = n;
			break;
		}
		case e_quiet:
			// check for 'no confirmation' option
			if (0 == _memicmp(core_args, "-q"/*uiet*/, 2))
				lParam = BBOPT_QUIET;
			break;

		case e_pause:
			// check for 'pause restart' option
			if (0 == _memicmp(core_args, "-p"/*ause*/, 2))
				lParam = BBOPT_PAUSE;
			break;

		case e_true: // ...AllWorkspaces option
			lParam = 1;
			break;

		case e_bool:
			wParam = 1 + get_false_true(core_args);
			break;

			// --- special (no message) commands ---
		case e_rootCommand:
			Desk_new_background(core_args);
			PostMessage(BBhwnd, BB_MENU, BB_MENU_UPDATE, 0);
			break;

		case e_Message:
			BBMessageBox(MB_OK, "%s", core_args);
			break;

		case e_ShowAppnames:
			ShowAppnames();
			break;

		case e_About:
			bb_about();
			break;

		case e_Nop:
			break;

		case e_Pause:
			BBSleep(atoi(core_args));
			break;

		case e_Crash:
			*(DWORD*)0 = 0x11111111;
			break;

		case e_ShowRecoverMenu:
			ShowRecoverMenu();
			break;

		case e_RecoverWindow:
		{
			HWND hwnd;
			if (sscanf(core_args, "%p", &hwnd))
				getWorkspaces().ToggleWindowVisibility(hwnd);
			break;
		}
		case e_Test:
			break;
		}

		if (msg)
		{
			// Some things need to be 'Post'ed, i.e. to return from plugins
			// before they are unloaded with restart, quit etc.
			if (action->m_flag & e_post)
				PostMessage(BBhwnd, msg, wParam, lParam);
			else
				SendMessage(BBhwnd, msg, wParam, lParam);
		}
		return 1;
	}

	/* ----------------------------------------------------------------------- */
	/* This is for the menu checkmarks in the styles and backgrounds folders */
	bool get_opt_command (char * opt_cmd, const char * cmd)
	{
		TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "cmd = %s", cmd);
		if (0 == opt_cmd[0])
		{
			if (0 == _memicmp(cmd, "@BBCore.", 8))
			{
				// internals, currently for style and rootcommand
#define CHECK_BROAM(broam) 0 == _memicmp(cmd, s = broam, sizeof broam - 3)
				const char *s;
				if (CHECK_BROAM(MM_STYLE_BROAM))
					sprintf(opt_cmd, s, getStylePath().c_str());
				else if (CHECK_BROAM(MM_THEME_BROAM))
					sprintf(opt_cmd, s, g_defaultrc_path[0] ? g_defaultrc_path : "default");
				else if (CHECK_BROAM(MM_ROOT_BROAM))
					sprintf(opt_cmd, s, Desk_extended_rootCommand(NULL));
				else
					return false;
#undef CHECK_BROAM
			}
			else if (0 == MessageManager_Send(BB_GETBOOL, (WPARAM)opt_cmd, (LPARAM)cmd))
				return false; // nothing found
			else if (opt_cmd[0] == 1)
			{
				opt_cmd[0] = 0; // recheck next time;
				return true;
			}
		}
		return opt_cmd[0] && 0 == _stricmp(opt_cmd, cmd);
	}

	//===========================================================================
	bool exec_script (const TCHAR * broam)
	{
		TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);
		const TCHAR * p = 0, *a = 0;
		TCHAR * s = 0;
		int n = 0, c = 0;
		if ('[' != skip_spc(&broam))
			return false;
		for (n = _tcslen(++broam); n && (c = broam[--n], IS_SPC(c));)
			;
		if (0 == n || ']' != c)
			return false;
		s = new_str_n(broam, n);
		for (p = s; 0 != (n = nexttoken(a, p, TEXT("|")));)
		{
			s[a - s + n] = 0;
			exec_command(a);
		}
		free(s);
		return true;
	}

	//===========================================================================
	// returns 'true' for done, 'false' for pass on to plugins
	bool exec_broam (const char * broam)
	{
		TRACE_SCOPE_MSG(trace::e_Debug, trace::CTX_BBCore, "broam = %s", broam);
		if (0 == _memicmp(broam, "@BBCore.", 8))
		{
			if (0 == exec_core_broam(broam))
				goto broam_error;
		}
		else if (0 == _memicmp(broam, "@BBCfg.", 7))
		{
			if (0 == exec_cfg_command(broam+7))
				goto broam_error;
		}
		else if (0==_memicmp(broam, "@Script", 7))
		{
			exec_script(broam + 7);
			return true;
		}
		else if (0==_stricmp(broam, "@BBHidePlugins"))
		{
			Menu_All_Toggle(g_PluginsHidden = true);
		}
		else if (0==_stricmp(broam, "@BBShowPlugins"))
		{
			Menu_All_Toggle(g_PluginsHidden = false);
		}
		return false;

	broam_error:
		BBMessageBox(MB_OK, NLS2("$Error_UnknownBroam$", "Error: Unknown Broadcast Message:\n%s"), broam);
		TRACE_MSG(trace::e_Error, trace::CTX_BBCore, "Error: Unknown Broadcast Message: %s", broam);
		return false;
	}

}