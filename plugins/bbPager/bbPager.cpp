/*
 ============================================================================
 Blackbox for Windows: BBPager
 ============================================================================
 Copyright © 2003-2009 nc-17@ratednc-17.com
 Copyright © 2008-2009 The Blackbox for Windows Development Team
 http://www.ratednc-17.com
 http://bb4win.sourceforge.net
 ============================================================================
*/
#include "bbPager.h"
#include "Settings.h"
#include <bbPlugin.h>
#include <vector>
#include <string>

// from Settings.cpp
extern std::vector<RECT> desktopRect;
extern char rcpath[MAX_PATH];

const int refresh_time = 1000;	// Automatic repaint time in milliseconds

const char szAppName [] = "bbPager";  // The name of our window class, etc.
const char szVersion [] = "bbPager 3.8";
const char szInfoVersion [] = "3.8";
const char szInfoAuthor [] = "NC-17|unkamunka|mojmir";
const char szInfoRelDate [] = "2014-08-22";
const char szInfoLink [] = "http://www.ratednc-17.com/";
const char szInfoEmail [] = "irc://irc.freenode.net/bb4win";
const char szInfoUpdateURL [] = "http://www.blackbox4windows.com";

//=====================================================================
RuntimeState g_RuntimeState;
RuntimeState & getRuntimeState () { return g_RuntimeState; } // "singleton"

// Blackbox messages we want to "subscribe" to:
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, BB_DESKTOPINFO, BB_WORKSPACE, BB_BRINGTOFRONT,
	BB_ADDTASK,	BB_REMOVETASK, BB_ACTIVETASK, BB_MINMAXTASK, BB_WINDOWSHADE, BB_WINDOWGROWHEIGHT,
	BB_WINDOWGROWWIDTH,	BB_WINDOWLOWER,	BB_REDRAWTASK, /*BB_MINIMIZE,*/ BB_WINDOWMINIMIZE, BB_WINDOWRAISE,
	BB_WINDOWMAXIMIZE, BB_WINDOWRESTORE, BB_WINDOWCLOSE,
	BB_TASKSUPDATE, // IMPORTANT for flashing.
	0};

// Desktop information
int desktopPressed = -1;
std::vector<std::string> desktopName;

// Window information
int winPressed = -1;
bool winMoving = false;
WinStruct moveWin;
int mx = 0, my = 0, oldx = 0, oldy = 0, grabDesktop = 0;
HWND lastActive;
bool passive = false;

// flashing tasks
std::vector<FlashTask> flashList;

// tooltips
HWND hToolTips;
bool tempTool;

// Mouse button settings
bool leftButtonDown = false, middleButtonDown = false,
rightButtonDown = false, rightButtonDownNC = false;

// Menus
Menu *BBPagerMenu, *BBPagerWindowSubMenu, *BBPagerDisplaySubMenu;
Menu *BBPagerPositionSubMenu, *BBPagerAlignSubMenu;
Menu *BBPagerSettingsSubMenu, *BBPagerDebugSubMenu;

// Slit items
int heightOld = 0, widthOld = 0, posXOld = 0, posYOld = 0;
int xpos, ypos;
bool inSlit = false;	//Are we loaded in the slit? (default of no)
HWND hSlit;				//The Window Handle to the Slit (for if we are loaded)

// Compatibility
bool (*BBPager_STL)(HWND, taskinfo const *, UINT) = NULL;
tasklist const * (*BBPager_GTLP)(void) = NULL;
tasklist const * tl = 0;
bool is_xoblite, usingAltMethod;

//bool loggerOn = true;

//===========================================================================

extern "C"
{
	DLL_EXPORT int beginPlugin(HINSTANCE hPluginInstance);
	DLL_EXPORT int beginSlitPlugin(HINSTANCE hPluginInstance, HWND hBBSlit);
	DLL_EXPORT int beginPluginEx(HINSTANCE hPluginInstance, HWND hBBSlit)
	{
		Settings & s = getSettings();
		hSlit = hBBSlit;
		WNDCLASS wc;
		g_RuntimeState.m_hwndBlackbox = GetBBWnd();
		g_RuntimeState.m_hInstance = hPluginInstance;

		*(FARPROC*)&BBPager_STL = GetProcAddress(GetModuleHandle(NULL), "SetTaskLocation");
		*(FARPROC*)&BBPager_GTLP = GetProcAddress(GetModuleHandle(NULL), "GetTaskListPtr");

		// Register the window class...
		ZeroMemory(&wc,sizeof(wc));
		wc.lpfnWndProc = WndProc;			// our window procedure
		wc.hInstance = hPluginInstance;		// hInstance of .dll
		wc.lpszClassName = szAppName;		// our window class name
		if (!RegisterClass(&wc))
		{
			MessageBox(g_RuntimeState.m_hwndBlackbox, "Error registering window class", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
			return 1;
		}

		// Get plugin and style settings...
		const char *bbv = GetBBVersion();
		g_RuntimeState.m_is_xoblite = 0 == (_memicmp(bbv, "bb", 2) + strlen(bbv) - 3);

		BBP_get_rcpath(rcpath, g_RuntimeState.m_hInstance, szAppName);

		s.ReadRCSettings();
		s.GetStyleSettings();

		inSlit = s.m_useSlit;
		tempTool = s.m_desktop.tooltips;

		// Set window size and position
		desktopName.clear();
		g_RuntimeState.m_winList.clear();
		desktopRect.clear();

		GetPos(false);
		SetPos(s.m_position.side);
		UpdatePosition();

		// Create the window...
		g_RuntimeState.m_hwndBBPager = CreateWindowEx(
			WS_EX_TOOLWINDOW,								// window style
			szAppName,										// our window class name
			NULL,											// NULL -> does not show up in task manager!
			WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,	// window parameters
			s.m_position.x,						// x position
			s.m_position.y,						// y position
			0,												// window width
			0,												// window height
			NULL,											// parent window
			NULL,											// no menu
			hPluginInstance,								// hInstance of .dll
			NULL);

		if (!g_RuntimeState.m_hwndBBPager)
		{
			MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
			return 1;
		}

		if (!hSlit)
			hSlit = FindWindow("BBSlit", "");

		if (inSlit && hSlit)		// Are we in the Slit?
		{
			if (s.m_position.autohide)
				s.m_position.autohideOld = true;
			else
				s.m_position.autohideOld = false;
			s.m_position.autohide = false;
			SendMessage(hSlit, SLIT_ADD, 0, (LPARAM)g_RuntimeState.m_hwndBBPager);

			/* A window can not be a WS_POPUP and WS_CHILD so remove POPUP and add CHILD
			* IF you decide to allow yourself to be unloaded from the slit, then you would
			* do the oppisite, remove CHILD and add POPUP
			*/
			SetWindowLong(g_RuntimeState.m_hwndBBPager, GWL_STYLE, (GetWindowLong(g_RuntimeState.m_hwndBBPager, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);

			// Make your parent window BBSlit
			SetParent(g_RuntimeState.m_hwndBBPager, hSlit);
		}
		else
		{
			s.m_useSlit = inSlit = false;
		}

		// Transparency is only supported under Windows 2000/XP...
		OSVERSIONINFO osInfo;
		ZeroMemory(&osInfo, sizeof(osInfo));
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		GetVersionEx(&osInfo);

		// @TODO: use general GetOSVersion.. i saw it somewhere
		if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osInfo.dwMajorVersion >= 5)
			getRuntimeState().m_usingWin2kXP = true;
		else
			getRuntimeState().m_usingWin2kXP = false;

		if (getRuntimeState().m_usingWin2kXP)
		{
			if (s.m_transparency && !inSlit)
				SetTransparency(g_RuntimeState.m_hwndBBPager, (unsigned char)s.m_transparencyAlpha);
		}

		// Register to receive Blackbox messages...
		SendMessage(g_RuntimeState.m_hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)g_RuntimeState.m_hwndBBPager, (LPARAM)msgs);
		MakeSticky(g_RuntimeState.m_hwndBBPager);
		// Make the window AlwaysOnTop?
		if (s.m_position.raised)
			SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

		// Set window size again, etc.
		getSettings().UpdateMonitorInfo();
		UpdatePosition();

		if (hSlit && inSlit)
		{
			SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOP, 0, 0, s.m_frame.width, s.m_frame.height, SWP_NOMOVE | SWP_NOZORDER);
			SendMessage(hSlit, SLIT_UPDATE, 0, 0);
		}

		if (!g_RuntimeState.m_is_xoblite)
			tl = BBPager_GetTaskListPtr();

		// Show the window and force it to update...
		ShowWindow(g_RuntimeState.m_hwndBBPager, SW_SHOW);
		InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);

		hToolTips = CreateWindowEx(	// Create tooltips window
			WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			hPluginInstance,
			NULL);

		if (!hToolTips)
		{
			MessageBox(0, "Error creating tooltips window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
			//Log("SysTray: Error creating tooltips window", NULL);
		}
		else	// set tooltip delay times based on MS Windows setting
		{
			SendMessage(hToolTips, TTM_SETMAXTIPWIDTH, 0, 300);
			SendMessage(hToolTips, TTM_SETDELAYTIME, TTDT_AUTOPOP, 5000/*GetDoubleClickTime()*10*/);
			SendMessage(hToolTips, TTM_SETDELAYTIME, TTDT_INITIAL, 120/*GetDoubleClickTime()*/);
			SendMessage(hToolTips, TTM_SETDELAYTIME, TTDT_RESHOW,  60/*GetDoubleClickTime()/5*/);
		}

		// Timer for auto redraw, to catch window movement
		if (!SetTimer(g_RuntimeState.m_hwndBBPager, 1, refresh_time, (TIMERPROC)NULL))
		{
			MessageBox(0, "Error creating update timer", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
			//return 0;
		}

		return 0;
	}

	//===========================================================================
	DLL_EXPORT void endPlugin(HINSTANCE hPluginInstance)
	{
		Settings & s = getSettings();
		KillTimer(g_RuntimeState.m_hwndBBPager, 1);

		// Delete used StyleItems...
		if (s.m_frame.style)
			delete s.m_frame.style;
		if (s.m_desktop.style)
			delete s.m_desktop.style; // @TODO: move to destructor
		if (s.m_activeDesktop.style)
			delete s.m_activeDesktop.style;
		if (s.m_window.style)
			delete s.m_window.style;
		if (s.m_focusedWindow.style)
			delete s.m_focusedWindow.style;

		// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
		if (BBPagerMenu)
			DelMenu(BBPagerMenu);

		// remove from slit
		if (inSlit)
			SendMessage(hSlit, SLIT_REMOVE, 0, (LPARAM)g_RuntimeState.m_hwndBBPager);

		// Unregister Blackbox messages...
		SendMessage(g_RuntimeState.m_hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)g_RuntimeState.m_hwndBBPager, (LPARAM)msgs);

		// Delete vectors
		desktopName.clear();
		g_RuntimeState.m_winList.clear();
		desktopRect.clear();
		flashList.clear();

		// Destroy our window...
		ClearToolTips();
		DestroyWindow(hToolTips);
		DestroyWindow(g_RuntimeState.m_hwndBBPager);

		// Unregister window class...
		UnregisterClass(szAppName, hPluginInstance);
	}

	//===========================================================================
	DLL_EXPORT LPCSTR pluginInfo(int field)
	{
		// pluginInfo is used by Blackbox for Windows to fetch information about
		// a particular plugin. At the moment this information is simply displayed
		// in an "About loaded plugins" MessageBox, but later on this will be
		// expanded into a more advanced plugin handling system. Stay tuned! :)
		switch (field)
		{
			case 1: return szAppName; // Plugin name
			case 2: return szInfoVersion; // Plugin version
			case 3: return szInfoAuthor; // Author
			case 4: return szInfoRelDate; // Release date, preferably in yyyy-mm-dd format
			case 5: return szInfoLink; // Link to author's website
			case 6: return szInfoEmail; // Author's email
			case 7:					//PLUGIN_BROAMS
				{
					return
						"@BBPager ListStuff"  //List other information
						"@BBPager ListDesktops"  //List desktops
						"@BBPager ListWindows"	//List windows
						"@BBPager ToggleWindows"  //Toggle window painting
						"@BBPager ToggleTrans"	//Toggles transparency
						"@BBPager ToggleNumbers"  //Toggles drawing of numbers on desktops
						"@BBPager OpenRC"  //Open bbpager.rc file with default editor
						"@BBPager OpenStyle"  //Open bbpager.bb file with default editor
						"@BBPager Vertical"  //Toggles vertical/horizontal alignment
						"@BBPager ToggleSnap"  //Toggle snap to windows
						"@BBPager ToggleRaised"  //Toggle always on top
						"@BBPager ToggleToolTips"  //Toggles showing tooltips
						"@BBPager ToggleHide"  //Toggles autohiding
						"@BBPager ToggleSlit"  //Toggle whether plugin is in slit
						"@BBPager Reload"  //Rereads RC|style settings and updates window
						"@BBPager LoadDocs"  //Loads bbPager documentation in browser
						"@BBPager About";  //Display message box about the plugin
				}
			case 8:		return szInfoUpdateURL; // Link to update page

				// ==========

			default:
				return szVersion; // Fallback: Plugin name + version, e.g. "MyPlugin 1.0"
		}
	}
}

//===========================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Settings & s = getSettings();
	switch (message)
	{
		// Window update process...
		case WM_PAINT:
		{
			UpdatePosition(); // get number of desktops and sets size/position of window
			DrawBBPager(hwnd);
			//ClearToolTips();
			return 0;
		}
		break;

		//====================
		case BB_TASKSUPDATE:
		{
			if (s.m_desktop.windows)
			{
				HWND taskHwnd = (HWND)wParam;

				if ((int)lParam == TASKITEM_MODIFIED)
				{
				}
				else if ((int)lParam == TASKITEM_ACTIVATED)
				{
					RemoveFlash(taskHwnd, false);
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				else if ((int)lParam == TASKITEM_ADDED)
				{
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				else if ((int)lParam == TASKITEM_FLASHED)
				{
					if (IsFlashOn(taskHwnd))
					{
						RemoveFlash(taskHwnd, true);
					}
					else if (taskHwnd != GetForegroundWindow())
					{
						AddFlash(taskHwnd);
						InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
					}
				}
				else if ((int)lParam == TASKITEM_REMOVED)
				{
					RemoveFlash(taskHwnd, false);
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
			}
		}
		break;

		// =================
		// Broadcast messages (bro@m -> the bang killah! :D <vbg>)
		case BB_BROADCAST:
		{
			char temp[64];
			strcpy(temp, (LPCSTR)lParam);
			
			// @BBShowPlugins: Show window and force update (global bro@m)
			if (!_stricmp(temp, "@BBShowPlugins"))
			{
				ShowWindow(g_RuntimeState.m_hwndBBPager, SW_SHOW);
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				return 0;
			}
			// @BBHidePlugins: Hide window (global bro@m)
			else if (!_stricmp(temp, "@BBHidePlugins"))
			{
				if (!inSlit)
					ShowWindow(g_RuntimeState.m_hwndBBPager, SW_HIDE);
				return 0;
			}
			// start positioning bro@ms for BBPager
			else if (IsInString(temp, "@BBPagerPosition"))
			{
				if (!inSlit)
				{
					char token1[32], token2[32], extra[32];
					LPSTR tokens[2];
					tokens[0] = token1;
					tokens[1] = token2;

					token1[0] = token2[0] = extra[0] = '\0';
					BBTokenize (temp, tokens, 2, extra);

					if (!_stricmp(token2, "TopLeft"))
						s.m_position.side = 3;
					else if (!_stricmp(token2, "TopCenter"))
						s.m_position.side = 2;
					else if (!_stricmp(token2, "TopRight"))
						s.m_position.side = 6;
					else if (!_stricmp(token2, "CenterLeft"))
						s.m_position.side = 1;
					else if (!_stricmp(token2, "CenterRight"))
						s.m_position.side = 4;
					else if (!_stricmp(token2, "BottomLeft"))
						s.m_position.side = 9;
					else if (!_stricmp(token2, "BottomCenter"))
						s.m_position.side = 8;
					else if (!_stricmp(token2, "BottomRight"))
						s.m_position.side = 12;
					else
						return 0;

					SetPos(s.m_position.side);

					SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOP, s.m_position.x, s.m_position.y, s.m_frame.width, s.m_frame.height, SWP_NOZORDER);
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);

					WriteString(rcpath, "bbPager.position:", token2);
				}
			}
			// start internal bro@ms for BBPager only
			else if (IsInString(temp, "@BBPager"))
			{
				char token1[32], token2[32], extra[32];
				LPSTR tokens[2];
				tokens[0] = token1;
				tokens[1] = token2;

				token1[0] = token2[0] = extra[0] = '\0';
				BBTokenize (temp, tokens, 2, extra);

				// @BBPagerReload rereads RC/style settings and updates window
				// Nice to do without reconfiguring all of blackbox
				if (!_stricmp(temp, "Reload"))
				{
					s.ReadRCSettings();
					s.GetStyleSettings();
					//UpdatePosition();
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
					return 0;
				}
				//open bbpager.rc file with default editor
				if (!_stricmp(token2, "OpenRC"))
				{
					BBExecute(GetDesktopWindow(), NULL, s.m_editor, rcpath, NULL, SW_SHOWNORMAL, false);
					return 0;
				}
				// open bbpager.bb file with default editor
				else if (!_stricmp(token2, "OpenStyle"))
				{
					BBExecute(GetDesktopWindow(), NULL, s.m_editor, getbspath(), NULL, SW_SHOWNORMAL, false);
					return 0;
				}
				// @BBPagerToggleNumbers toggles drawing of numbers on desktops
				else if (!_stricmp(token2, "ToggleNumbers"))
				{
					if (s.m_desktop.numbers)
						s.m_desktop.numbers = false;
					else
						s.m_desktop.numbers = true;

					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				// @BBPagerVertical / Horizontal toggles vertical/horizontal alignment
				// also forces window to update
				else if (!_stricmp(token2, "Vertical"))
				{
					if (s.m_position.horizontal) // set drawing to use columns/vertical
					{
						s.m_position.horizontal = false;
						s.m_position.vertical = true;
					}
					else
					{
						s.m_position.horizontal = true;
						s.m_position.vertical = false;
					}
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				else if (!_stricmp(token2, "Horizontal"))
				{
					if (s.m_position.vertical) // set drawing to use rows/horizontal
					{
						s.m_position.horizontal = true;
						s.m_position.vertical = false;
					}
					else
					{
						s.m_position.horizontal = false;
						s.m_position.vertical = true;
					}
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				 // @BBPagerToggleSnap toggles snap to windows and sets toolbar msg
				else if (!_stricmp(token2, "ToggleSnap"))
				{
					if (!inSlit)
					{
						if (s.m_position.snapWindow)
							s.m_position.snapWindow = false;
						else
							s.m_position.snapWindow = true;
					}
				}
				// @BBPagerToggleRaised toggles always on top and sets toolbar msg
				else if (!_stricmp(token2, "ToggleRaised"))
				{
					if (s.m_position.raised)
					{
						s.m_position.raised = false;
						SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
					}
					else
					{
						s.m_position.raised = true;
						SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
					}
				}
				// @BBPager ToggleHide toggles autohiding
				else if (!_stricmp(token2, "ToggleHide"))
				{
					if (!inSlit)
					{
						if (s.m_position.autohide)
						{
							s.m_position.autohide = false;
							s.m_position.hidden = false;
						}
						else
						{
							s.m_position.autohide = true;
							GetPos(true);
							SetPos(s.m_position.side);
							HidePager();
						}
					}
				}
				// @BBPagerToggleWindows toggles window painting
				else if (!_stricmp(token2, "ToggleWindows"))
				{
					if (s.m_desktop.windows)
						s.m_desktop.windows = false;
					else
						s.m_desktop.windows = true;

					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				else if (!_stricmp(token2, "ToggleToolTips"))
				{
					if (s.m_desktop.tooltips)
					{
						s.m_desktop.tooltips = false;
						ClearToolTips();
					}
					else
					{
						s.m_desktop.tooltips = true;
					}
					tempTool = s.m_desktop.tooltips;

					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				else if (!_stricmp(token2, "ToggleAltMethod") && !g_RuntimeState.m_is_xoblite)
				{
					if (g_RuntimeState.m_usingAltMethod)
						g_RuntimeState.m_usingAltMethod = false;
					else
						g_RuntimeState.m_usingAltMethod = true;

					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				else if (!_stricmp(token2, "ToggleBorder"))
				{
					if (s.m_drawBorder)
						s.m_drawBorder = false;
					else
						s.m_drawBorder = true;

					s.GetStyleSettings();
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				// @BBPager Transparency toggles transparency in w2k/xp
				else if (!_stricmp(token2, "ToggleTrans"))
				{
					if (getRuntimeState().m_usingWin2kXP)
					{
						if (s.m_transparency)
						{
							s.m_transparency = false;
							SetTransparency(g_RuntimeState.m_hwndBBPager, 255);
						}
						else if (!inSlit)
						{
							s.m_transparency = true;
							SetTransparency(g_RuntimeState.m_hwndBBPager, (unsigned char)s.m_transparencyAlpha);
						}
					}
				}
				else if (!_stricmp(token2, "ToggleSlit"))
				{
					ToggleSlit();
					s.WriteRCSettings();
					return 0;
				}
				else if (!_stricmp(token2, "LoadDocs"))
				{
					char docspath[MAX_PATH];
					strcpy(docspath, rcpath);
					char *target_file = strrchr(docspath, '\\');
					if (target_file) ++target_file;
					strcpy(target_file, "bbPager.html");
					ShellExecute(g_RuntimeState.m_hwndBlackbox, "open", docspath, NULL, NULL, SW_SHOWNORMAL);
				}
				// @BBPagerAbout pops up info box about the plugin
				else if (!_stricmp(token2, "About"))
				{
					char tempaboutmsg[MAX_PATH];

					sprintf(tempaboutmsg, "%s\n\n"
					"© 2003 nc-17@ratednc-17.com\n\n"
					"http://www.ratednc-17.com/\n"
					"#bb4win on irc.freenode.net", szVersion);
					BBMessageBox(MB_OK, tempaboutmsg);
					return 0;
				}

				s.WriteRCSettings();
			}
			return 0;
		}
		break;

		// =================
		// If Blackbox sends a reconfigure message, load the new style settings and update the window...
		case BB_RECONFIGURE:
		{
			s.ReadRCSettings();
			s.GetStyleSettings();
			UpdatePosition();

			getSettings().UpdateMonitorInfo();
			if (!inSlit)
			{
				// snap to edge on style change
				// stops diff. bevel/border widths moving pager from screen edge
				int x; int y; int z; int dx, dy, dz;
				int nDist = 20;

				// top/bottom edge
				dy = y = s.m_position.y - s.m_screenTop;
				dz = z = s.m_position.y + s.m_frame.height - s.m_screenBottom;
				if (dy<0) dy=-dy;
				if (dz<0) dz=-dz;
				if (dz < dy) y = z, dy = dz;

				// left/right edge
				dx = x = s.m_position.x - s.m_screenLeft;
				dz = z = s.m_position.x + s.m_frame.width - s.m_screenRight;
				if (dx<0) dx=-dx;
				if (dz<0) dz=-dz;
				if (dz < dx) x = z, dx = dz;

				if (dy < nDist)
				{
					s.m_position.y -= y;
					// top/bottom center
					dz = z = s.m_position.x + (s.m_frame.width - s.m_screenRight - s.m_screenLeft) / 2;
					if (dz<0) dz=-dz;
					if (dz < dx) x = z, dx = dz;
				}

				if (dx < nDist)
					s.m_position.x -= x;

				SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOP, s.m_position.x, s.m_position.y, s.m_frame.width, s.m_frame.height, SWP_NOZORDER);
			}

			//UpdatePosition();

			InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
		}
		break;

		// ===================
		// Grabs the number of desktops and sets the names of the desktops
		case BB_DESKTOPINFO:
		{
			DesktopInfo* info = (DesktopInfo*)lParam;
			if (info->isCurrent)
			{
				g_RuntimeState.m_currentDesktop = info->number;
			}
			//strcpy(desktopName[desktops], info->name); // Grab the name of each desktop as it comes
			desktopName.push_back(std::string(info->name));
			g_RuntimeState.m_desktops++; // Increase desktop count by one for each
		}
		break;

		//====================
		// This is done for workspace switching with the toolbar
		// as a result of focusing an app that is on another workspace
		case BB_BRINGTOFRONT:

		// Force window to update when workspaces are changed or added, etc.
		// Or when apps moved by middle click on taskbar/desktop
		case BB_WORKSPACE:
		case BB_LISTDESKTOPS:

		// Here's all the Hook msgs, makes the pager a lot more responsive to window changes
		case BB_ADDTASK:		case BB_REMOVETASK:		case BB_ACTIVETASK:
		case BB_MINMAXTASK:		case BB_WINDOWSHADE:	case BB_WINDOWGROWHEIGHT:
		case BB_WINDOWGROWWIDTH:case BB_WINDOWLOWER:	case BB_REDRAWTASK:
		/*case BB_MINIMIZE:*/	case BB_WINDOWMINIMIZE:	case BB_WINDOWRAISE:
		case BB_WINDOWMAXIMIZE:	case BB_WINDOWRESTORE:	case BB_WINDOWCLOSE:
		{
			InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
		}
		break;

		// ==================		
		case WM_WINDOWPOSCHANGING:
		{
			if (!inSlit)
			// Snap window to screen edges (if the last bool is false it uses the current DesktopArea)
				if (IsWindowVisible(hwnd))
					SnapWindowToEdge((WINDOWPOS*)lParam, s.m_position.snapWindow, true);
			return 0;
		}
		break;

		// ==================
		// Save window position if it changes...
		case WM_WINDOWPOSCHANGED:
		{
			if (!inSlit)
			{
				WINDOWPOS* windowpos = (WINDOWPOS*)lParam;

				s.m_position.x = windowpos->x;
				s.m_position.y = windowpos->y;

				WriteInt(rcpath, "bbPager.position.x:", s.m_position.x);
				WriteInt(rcpath, "bbPager.position.y:", s.m_position.y);
			}
			return 0;
		}
		break;

		// ==================
		case WM_MOVING:
		{
			RECT *r = (RECT *)lParam;

			if (!s.m_position.autohide)
			{
				// Makes sure BBPager stays on screen when moving.
				if (r->left < s.m_vScreenLeft)
				{
					r->left = s.m_vScreenLeft;
					r->right = r->left + s.m_frame.width;
				}
				if ((r->left + s.m_frame.width) > s.m_vScreenRight)
				{
					r->left = s.m_vScreenRight - s.m_frame.width;
					r->right = r->left + s.m_frame.width;
				}

				if (r->top < s.m_vScreenTop)
				{
					r->top = s.m_vScreenTop;
					r->bottom = r->top + s.m_frame.height;
				}
				if ((r->top + s.m_frame.height) > s.m_vScreenBottom)
				{
					r->top = s.m_vScreenBottom - s.m_frame.height;
					r->bottom = r->top + s.m_frame.height;
				}
			}
			return TRUE;
		}
		break;

		// ==================
		case WM_DISPLAYCHANGE:
		{
			// IntelliMove(tm) by qwilk... <g>
			int const oldscreenwidth = s.m_screenWidth;
			int const oldscreenheight = s.m_screenHeight;

			getSettings().UpdateMonitorInfo();

			if (s.m_position.x > oldscreenwidth / 2)
			{
				int const relx = oldscreenwidth - s.m_position.x;
				s.m_position.x = s.m_screenRight - relx;
			}

			if (s.m_position.y > oldscreenheight / 2)
			{
				int const rely = oldscreenheight - s.m_position.y;
				s.m_position.y = s.m_screenBottom - rely;
			}

			if (!inSlit)
				UpdatePosition();
		}
		break;

		// ==================
		// Allow window to move...
		case WM_NCHITTEST:
		{
			ClickMouse();

			if (/*(desktopPressed == -1) || */(GetAsyncKeyState(VK_CONTROL) & 0x8000))
			{
				if (!s.m_position.autohide)
					return HTCAPTION;
				else
					return HTCLIENT;
			}
			else
			{
				return HTCLIENT;
			}
		}
		break;

		// ===================
		// Left mouse button msgs
		case WM_NCLBUTTONDBLCLK:
		case WM_LBUTTONDBLCLK:
		{
			ToggleSlit();
			break;
		}
		case WM_NCLBUTTONDOWN:
		{
			/* Please do not allow plugins to be moved in the slit.
			 * That's not a request..  Okay, so it is.. :-P
			 * I don't want to hear about people losing their plugins
			 * because they loaded it into the slit and then moved it to
			 * the very edge of the slit window and can't get it back..
			 */
			if (!inSlit || !hSlit)
				return DefWindowProc(hwnd, message, wParam, lParam);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			leftButtonDown = true;
			if (s.m_moveButton == 1)
				if (!winMoving)
					GrabWindow();
			return 0;
		}
		case WM_LBUTTONUP:
		{
			if (leftButtonDown)
			{
				if (s.m_focusButton == 1 && !winMoving) // if window focus button is RMB
					FocusWindow();

				if (s.m_moveButton == 1 && winMoving)
				{
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						passive = true;
					DropWindow();
					winMoving = false;
					s.m_desktop.tooltips = tempTool;
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				
				if (s.m_desktopChangeButton == 1 && !winMoving)
				{
					if (desktopPressed != g_RuntimeState.m_currentDesktop)
						DeskSwitch();
				}
			}
			leftButtonDown = false;
			return 0;
		}

		// ===================
		// Middle mouse button msgs
		case WM_MBUTTONDOWN:
		{
			middleButtonDown = true;
			if (s.m_moveButton == 3)
				if (!winMoving)
					GrabWindow();
			return 0;
		}
		case WM_MBUTTONUP:
		{
			if (middleButtonDown)
			{
				if (s.m_focusButton == 3 && !winMoving) // if window focus button is RMB
				{
					FocusWindow();
					//return 0;
				}
				if (s.m_moveButton == 3 && winMoving)
				{
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						passive = true;
					DropWindow();
					winMoving = false;
					s.m_desktop.tooltips = tempTool;
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}
				if (s.m_desktopChangeButton == 3 && !winMoving)
				{
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						ToggleSlit();
					else
						if (desktopPressed != g_RuntimeState.m_currentDesktop)
						DeskSwitch();
					//return 0;
				}
			}
			middleButtonDown = false;
			return 0;
		}
		case WM_NCMBUTTONUP:
		{
			if (winMoving && middleButtonDown)
			{
				DropWindow();
				winMoving = false;
				s.m_desktop.tooltips = tempTool;
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			}
			middleButtonDown = false;
			break;
		}

		// ====================
		// Right mouse button msgs
		case WM_NCRBUTTONDOWN:
		{
			rightButtonDownNC = true; // right button clicked on BBPager
			return 0;
		}
		case WM_NCRBUTTONUP:
		{
			if (rightButtonDownNC) // was right button clicked on BBPager?
			{
				DisplayMenu(); // Just show menu if right clicked on Non-Client Area (not desktops)
				rightButtonDownNC = false;
			}
			if (winMoving)
			{
				winMoving = false;
				s.m_desktop.tooltips = tempTool;
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			}
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			rightButtonDown = true; // right button clicked on BBPager
			if (s.m_moveButton == 2)
				if (!winMoving)
					GrabWindow();
			return 0;
		}
		case WM_RBUTTONUP:
		{
			if (rightButtonDown) // was right button clicked on BBPager?
			{
				if (/*desktopPressed == -1 || */(GetAsyncKeyState(VK_CONTROL) & 0x8000))
				{	// if mouse was not inside a desktop, show the menu
					DisplayMenu();
					return 0;
				}

				if ((s.m_focusButton == 2 || (s.m_focusButton == 3 && (GetAsyncKeyState(VK_SHIFT) & 0x8000))) && !winMoving) // if window focus button is RMB
					FocusWindow();

				if ((s.m_moveButton == 2 || (s.m_moveButton == 3 && (GetAsyncKeyState(VK_SHIFT) & 0x8000))) && winMoving)
				{
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						passive = true;
					DropWindow();
					winMoving = false;
					s.m_desktop.tooltips = tempTool;
					InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				}

				if ((s.m_desktopChangeButton == 2 || (s.m_desktopChangeButton == 3 && (GetAsyncKeyState(VK_SHIFT) & 0x8000))) && !winMoving) // if desktop switch button is RMB
				{
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
						ToggleSlit();
					else
						if (desktopPressed != g_RuntimeState.m_currentDesktop)
							DeskSwitch();
					else
					// if desktop switch button is not RMB, just show menu
						DisplayMenu();
				}

				rightButtonDown = false;
			}
			return 0;
		}

		// ===================
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		{				
			if (s.m_position.hidden && s.m_position.autohide)
			{
				s.m_position.hidden = false;
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				//Sleep(250);
				s.m_position.x = s.m_position.ox;
				s.m_position.y = s.m_position.oy;
				// reverted to original code - otherwise written to RC
				MoveWindow(g_RuntimeState.m_hwndBBPager, s.m_position.ox, s.m_position.oy, s.m_frame.width, s.m_frame.height, true);
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			}

			if (winMoving)
			{
				int nx, ny, dx, dy;
				RECT r;
				POINT mousepos;
				GetCursorPos(&mousepos);
				GetWindowRect(g_RuntimeState.m_hwndBBPager, &r);

				//mousepos.x -= position.x;
				//mousepos.y -= position.y;
				mousepos.x -= r.left;
				mousepos.y -= r.top;

				nx = mousepos.x - moveWin.r.left;
				ny = mousepos.y - moveWin.r.top;

				dx = nx - mx;
				dy = ny - my;

				moveWin.r.left += dx;
				moveWin.r.right += dx;
				moveWin.r.top += dy;
				moveWin.r.bottom += dy;

				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			}
			else
				lastActive = GetForegroundWindow();

			TrackMouse();

			return 0;
		}
		break;

		// ===================
		// reset pressed mouse button statuses if mouse leaves the BBPager window
		case WM_MOUSELEAVE:
		{
			if (winMoving)
			{
				if (s.m_moveButton != 1)
					leftButtonDown = false;
				if (s.m_moveButton != 2)
					rightButtonDown = false;
				if (s.m_moveButton != 3)
					middleButtonDown = false;
			}
			else
				leftButtonDown = middleButtonDown = rightButtonDown = false;

			if (s.m_position.autohide && !inSlit)
				HidePager();

			if (CursorOutside() && winMoving)
			{
				winMoving = leftButtonDown = middleButtonDown = rightButtonDown = false;
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			}
			return 0;
		}
		case WM_NCMOUSELEAVE:
		{
			rightButtonDownNC = false;

			if (s.m_position.autohide && !inSlit)
				HidePager();

			if (CursorOutside() && winMoving)
			{
				winMoving = leftButtonDown = middleButtonDown = rightButtonDown = false;
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			}
			return 0;
		}

		//====================
		// Mouse wheel desktop changing:
		case WM_MOUSEWHEEL:
		{
			int delta = (int)wParam;

			if (delta < 0)
				SendMessage(g_RuntimeState.m_hwndBlackbox, BB_WORKSPACE, 1, 0);
			else
				SendMessage(g_RuntimeState.m_hwndBlackbox, BB_WORKSPACE, 0, 0);

			SetForegroundWindow(g_RuntimeState.m_hwndBBPager);
			return 0;
		}

		//====================
		// Timer message:
		case WM_TIMER:
		{
			if (CursorOutside() && winMoving)
			{
				winMoving = false;
				s.m_desktop.tooltips = tempTool;
			}

			InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			return 0;
		}

		// ===================
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE && winMoving)
			{
				winMoving = false;
				s.m_desktop.tooltips = tempTool;
			}
			
			InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			return 0;
		}

		case WM_KEYUP:
		{
			if (wParam == VK_NUMPAD7)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition TopLeft");
			else if (wParam == VK_NUMPAD8)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition TopCenter");
			else if (wParam == VK_NUMPAD9)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition TopRight");
			else if (wParam == VK_NUMPAD4)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition CenterLeft");
			else if (wParam == VK_NUMPAD6)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition CenterRight");
			else if (wParam == VK_NUMPAD1)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition BottomLeft");
			else if (wParam == VK_NUMPAD2)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition BottomCenter");
			else if (wParam == VK_NUMPAD3)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerPosition BottomRight");
			else if (wParam == VK_BACK || wParam == VK_RETURN)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPager ToggleHide");
			else if (wParam == VK_SUBTRACT)
				SendMessage(g_RuntimeState.m_hwndBBPager, BB_BROADCAST, 0, (LPARAM)"@BBPagerReload");
			else if (wParam == VK_ESCAPE && winMoving)
			{
				winMoving = false;
				s.m_desktop.tooltips = tempTool;
			}
			
			InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			return 0;
		}

		case WM_CLOSE:
			return 0;

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
		break;
	}
	return 0;
}

//===========================================================================
void ToggleSlit()
{
	Settings const & s = getSettings();
	/*static char msg[MAX_PATH];*/
	static char status[9];

	if (!hSlit) hSlit = FindWindow("BBSlit", "");

	if (inSlit)
	{
		// We are in the slit, so lets unload and get out..
		if (getSettings().m_position.autohideOld)
			getSettings().m_position.autohide = true;
		SendMessage(hSlit, SLIT_REMOVE, 0, (LPARAM)g_RuntimeState.m_hwndBBPager);
		inSlit = false;

		//turn trans back on?
		if (s.m_transparency && getRuntimeState().m_usingWin2kXP)
		{
			SetWindowLong(g_RuntimeState.m_hwndBBPager, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
			SetTransparency(g_RuntimeState.m_hwndBBPager, (unsigned char)s.m_transparencyAlpha);
		}

		getSettings().m_position.x = xpos;
		getSettings().m_position.y = ypos;

		// Here you can move to where ever you want ;)
		if (getSettings().m_position.raised)
			SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOPMOST, xpos, ypos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		else
			SetWindowPos(g_RuntimeState.m_hwndBBPager, NULL, xpos, ypos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
	/* Make sure before you try and load into the slit that you have
	 * the HWND of the slit ;)
	 */
	else if (hSlit)
	{
		// (Back) into the slit..
		if (getSettings().m_position.autohide)
			getSettings().m_position.autohideOld = true;
		else
			getSettings().m_position.autohideOld = false;
		getSettings().m_position.autohide = false;
		xpos = getSettings().m_position.x;
		ypos = getSettings().m_position.y;

		//turn trans off
		SetWindowLong(g_RuntimeState.m_hwndBBPager, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

		//SetParent(hwndBBPager, hSlit);
		//SetWindowLongPtr(hwndBBPager, GWL_STYLE, (GetWindowLongPtr(hwndBBPager, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);
		SendMessage(hSlit, SLIT_ADD, 0, (LPARAM)g_RuntimeState.m_hwndBBPager);
		inSlit = true;
	}

	if (hSlit)
	{
		(inSlit) ? strcpy(status, "enabled") : strcpy(status, "disabled");
		/*sprintf(msg, "bbPager -> Use Slit %s", status);
		SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)msg);*/
		WriteBool(rcpath, "bbPager.useSlit:", inSlit);
	}
}

//===========================================================================
// Grabs handles to windows, checks them, and saves their info in an array of structs
BOOL CALLBACK CheckTaskEnumProc(HWND window, LPARAM lParam)
{
	Settings const & s = getSettings();
	int d = 0, height = 0, width = 0, offsetX = 0;
	RECT desk;
	desk.top = desk.left = 0;
	WinStruct winListTemp;

	// Only allow 128 windows, just for speed reasons...
	if (g_RuntimeState.m_winCount > 128)
		return 0;

	// Make sure the window meets criteria to be drawn
	if (IsValidWindow(window))
	{
		// Get the virtual workspace the window is located.
		d = getDesktop(window);
		if (d < 0 || d >= desktopRect.size())
			return 0;

		// ... and save this information
		winListTemp.desk = d;

		if (CheckSticky(window))
			winListTemp.sticky = true;
		else
			winListTemp.sticky = false;

		if (getSettings().m_position.grid)
			desk = desktopRect[d];
		else if (getSettings().m_position.horizontal)
			desk = desktopRect[d];
		else if (getSettings().m_position.vertical)
			desk = desktopRect[d];

		// Check whether the window is currently active/focused
		HWND temp = GetForegroundWindow();
		if (temp == window)
			winListTemp.active = true;
		else
			winListTemp.active = false;

		// Set window handle
		winListTemp.window = window;

		// Get window coordinates/RECT
		GetWindowRect(window, &winListTemp.r);

		// offset in desktops away from current desktop
		offsetX = g_RuntimeState.m_currentDesktop - winListTemp.desk;

		// Height and width of window as displayed in the pager
		height = int(1 + (winListTemp.r.bottom - winListTemp.r.top) / s.m_ratioY);
		width = int(1 + (winListTemp.r.right - winListTemp.r.left) / s.m_ratioX);
		
		// Set RECT of window using width/height and top/left values
		winListTemp.r.top = long(desk.top + winListTemp.r.top / s.m_ratioY);

		//winList[winCount].r.bottom = long( winList[winCount].r.top + (winList[winCount].r.bottom - winList[winCount].r.top) /  ratioY );
		winListTemp.r.bottom = winListTemp.r.top + height;

		while (winListTemp.r.left < -10 + s.m_vScreenLeft)
		{
			winListTemp.r.left += s.m_vScreenWidth + 10;
		}

		while (winListTemp.r.left >= s.m_vScreenWidth + s.m_vScreenLeft)
		{
			winListTemp.r.left -= s.m_vScreenWidth + 10;
		}

		winListTemp.r.left -= s.m_vScreenLeft;
		winListTemp.r.left = long(desk.left + (winListTemp.r.left / s.m_ratioX));

		//winList[winCount].r.right = long( winList[winCount].r.left + (winList[winCount].r.right - winList[winCount].r.left) /  ratioX );
		winListTemp.r.right = winListTemp.r.left + width;

		g_RuntimeState.m_winList.push_back(winListTemp);
		//winList[winCount] = winListTemp;
		// Increase number of windows by one if everything successful
		g_RuntimeState.m_winCount++;
	}

	UNREFERENCED_PARAMETER( lParam );
	return 1;
}

BOOL CALLBACK CheckTaskEnumProc_AltMethod (HWND window, LPARAM lParam)
{
	tl = BBPager_GetTaskListPtr();

	while (tl)
	{
		if (tl->hwnd == window)
		{
			AddBBWindow(tl);
			return 1;
		}
		tl = tl->next;
	}

	/*tasklist tl2;
	tl2.hwnd = window;

	if (DoWindow(&tl2))
	{
		AddBBWindow(&tl2);
	}*/

	return 1;
}

bool AddBBWindow(tasklist const *tl)
{
	Settings const & s = getSettings();
	HWND window = tl->hwnd;

	// Only allow 128 windows, just for speed reasons...
	if (g_RuntimeState.m_winCount > 128)
		return 0;

	WinStruct winListTemp;
	// Get the virtual workspace the window is located.
	int const bbdesk = tl->wkspc;
	// ... and save this information
	winListTemp.desk = bbdesk;

	RECT desk = desktopRect[bbdesk];

	if (CheckSticky(window))
		winListTemp.sticky = true;
	else
		winListTemp.sticky = false;

	winListTemp.active = tl->active;
	winListTemp.window = window;// Set window handle

	// Get window coordinates/RECT
	GetWindowRect(window, &winListTemp.r);

	// Height and width of window as displayed in the pager
	int const height = int(1 + (winListTemp.r.bottom - winListTemp.r.top) / s.m_ratioY);
	int const width = int(1 + (winListTemp.r.right - winListTemp.r.left) / s.m_ratioX);
	
	// Set RECT of window using width/height and top/left values
	winListTemp.r.top = long(desk.top + winListTemp.r.top / s.m_ratioY);
	winListTemp.r.bottom = winListTemp.r.top + height;
	winListTemp.r.left = long(desk.left + (winListTemp.r.left / s.m_ratioX));
	winListTemp.r.right = winListTemp.r.left + width;

	g_RuntimeState.m_winList.push_back(winListTemp);
	g_RuntimeState.m_winCount++; // Increase number of windows by one if everything successful
	return 1;
}

//===========================================================================
bool IsValidWindow (HWND hWnd)
{
	// if window is minimised, fail it
	// (no point in displaying something not there =)
	if (IsIconic(hWnd))
		return false;

	// if it is a WS_CHILD or not WS_VISIBLE, fail it
	LONG nStyle = GetWindowLong(hWnd, GWL_STYLE);	
	if ((nStyle & WS_CHILD) || !(nStyle & WS_VISIBLE))
		return false;
	
	// *** below is commented out because trillian is a toolwindow
	// *** and perhaps miranda (and other systray things?)
	// if the window is a WS_EX_TOOLWINDOW fail it
	nStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	if ((nStyle & WS_EX_TOOLWINDOW) && (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP))
		return false;

	// if it has a parent, fail
	if (GetParent(hWnd))
		return false;
	
	return true;
}

//===========================================================================
int getDesktop (HWND h)
{
	if (g_RuntimeState.m_desktops < 1)
		return -1;

	Settings const & s = getSettings();
	RECT r;
	GetWindowRect(h, &r);
	if (r.left == -32000 || r.top == -32000)
		return -1; // this means minimized window

	int const desktopWidth = s.m_vScreenWidth + 10;
	int offset = 0;
	int winLeft = r.left;

	while (winLeft < -10 + s.m_vScreenLeft)
	{
		winLeft += desktopWidth;
		offset--;
	}

	while (winLeft >= (s.m_vScreenWidth + s.m_vScreenLeft))
	{
		winLeft -= desktopWidth;
		offset++;
	}

	return offset + g_RuntimeState.m_currentDesktop;
}

//===========================================================================
// The actual building of the menu
void DisplayMenu()
{
	Settings const & s = getSettings();
	// First we delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (BBPagerMenu)
		DelMenu(BBPagerMenu);

	// Then we create the main plugin menu with a few commands...
	BBPagerMenu = MakeMenu("bbPager");

		// Window Submenu
		BBPagerWindowSubMenu = MakeMenu("Window");
			if (!inSlit)
			{
				MakeMenuItem(BBPagerWindowSubMenu, "Always On Top", "@BBPager ToggleRaised", getSettings().m_position.raised);
				MakeMenuItem(BBPagerWindowSubMenu, "Autohide", "@BBPager ToggleHide", getSettings().m_position.autohide);
				MakeMenuItem(BBPagerWindowSubMenu, "Snap Window To Edge", "@BBPager ToggleSnap", getSettings().m_position.snapWindow == 1);
				if (getRuntimeState().m_usingWin2kXP)
				{
					MakeMenuItem(BBPagerWindowSubMenu, "Transparency", "@BBPager ToggleTrans", s.m_transparency);
				}
			}
			/*MakeMenuItem(BBPagerWindowSubMenu, "Draw Border", "@BBPager ToggleBorder", drawBorder);*/
			if (!hSlit) hSlit = FindWindow("BBSlit", "");
			if (hSlit) MakeMenuItem(BBPagerWindowSubMenu, "Use Slit", "@BBPager ToggleSlit", inSlit);
		MakeSubmenu(BBPagerMenu, BBPagerWindowSubMenu, "Window");

		// Display Submenu
		BBPagerDisplaySubMenu = MakeMenu("Display");
			MakeMenuItem(BBPagerDisplaySubMenu, "Desktop Numbers", "@BBPager ToggleNumbers", getSettings().m_desktop.numbers);
			MakeMenuItem(BBPagerDisplaySubMenu, "Desktop Windows", "@BBPager ToggleWindows", getSettings().m_desktop.windows);
			MakeMenuItem(BBPagerDisplaySubMenu, "Window ToolTips", "@BBPager ToggleToolTips", getSettings().m_desktop.tooltips);
			if (!g_RuntimeState.m_is_xoblite)
				MakeMenuItem(BBPagerDisplaySubMenu, "Use Alt. Method", "@BBPager ToggleAltMethod", g_RuntimeState.m_usingAltMethod);
			//MakeMenuItem(BBPagerDisplaySubMenu, "Debugging", "@BBPager ToggleDebug", debug);
		MakeSubmenu(BBPagerMenu, BBPagerDisplaySubMenu, "Display");

		// Alignment Submenu
		BBPagerAlignSubMenu = MakeMenu("Alignment");
		    if (getSettings().m_position.grid)
			{
				//MakeMenuItem(BBPagerAlignSubMenu, "Horizontal", "@BBPager Horizontal", getSettings().m_position.horizontal);
			}
			else
			{
				MakeMenuItem(BBPagerAlignSubMenu, "Horizontal", "@BBPager Horizontal", getSettings().m_position.horizontal);
				MakeMenuItem(BBPagerAlignSubMenu, "Vertical", "@BBPager Vertical", getSettings().m_position.vertical);
			}
		MakeSubmenu(BBPagerMenu, BBPagerAlignSubMenu, "Alignment");		
		
		// Position Submenu
		if (!inSlit)
		{
			BBPagerPositionSubMenu = MakeMenu("Placement");
				MakeMenuItem(BBPagerPositionSubMenu, "Top Left", "@BBPagerPosition TopLeft", (getSettings().m_position.side == 3));
				MakeMenuItem(BBPagerPositionSubMenu, "Top Center", "@BBPagerPosition TopCenter", (getSettings().m_position.side == 2));
				MakeMenuItem(BBPagerPositionSubMenu, "Top Right", "@BBPagerPosition TopRight", (getSettings().m_position.side == 6));
				MakeMenuItem(BBPagerPositionSubMenu, "Center Left", "@BBPagerPosition CenterLeft", (getSettings().m_position.side == 1));
				MakeMenuItem(BBPagerPositionSubMenu, "Center Right", "@BBPagerPosition CenterRight", (getSettings().m_position.side == 4));
				MakeMenuItem(BBPagerPositionSubMenu, "Bottom Left", "@BBPagerPosition BottomLeft", (getSettings().m_position.side == 9));
				MakeMenuItem(BBPagerPositionSubMenu, "Bottom Center", "@BBPagerPosition BottomCenter", (getSettings().m_position.side == 8));
				MakeMenuItem(BBPagerPositionSubMenu, "Bottom Right", "@BBPagerPosition BottomRight", (getSettings().m_position.side == 12));
			MakeSubmenu(BBPagerMenu, BBPagerPositionSubMenu, "Placement");		
		}

		// Settings Submenu
		BBPagerSettingsSubMenu = MakeMenu("Settings");
			MakeMenuItem(BBPagerSettingsSubMenu, "Edit Settings", "@BBPager OpenRC", false);
			MakeMenuItem(BBPagerSettingsSubMenu, "Edit Style", "@BBPager OpenStyle", false);
		MakeSubmenu(BBPagerMenu, BBPagerSettingsSubMenu, "Settings");

	MakeMenuItem(BBPagerMenu, "Documentation", "@BBPager LoadDocs", false);
	MakeMenuItem(BBPagerMenu, "About BBPager...", "@BBPager About", false);
	
	// Finally, we show the menu...
	ShowMenu(BBPagerMenu);
}

//===========================================================================
// Checks to see if the cursor is inside the pager window
bool CursorOutside ()
{
	POINT pt;
	GetCursorPos(&pt);
	RECT r;
	GetWindowRect(g_RuntimeState.m_hwndBBPager, &r);

	//RECT r = {position.x, position.y, position.x + frame.width, position.y + frame.height};

	if (PtInRect(&r, pt))
		return false;
	else
		return true;
}

//===========================================================================
// This checks the coords of the click to set the appropriate desktop to pressed...
void ClickMouse ()
{
	RECT r;
	int inDesk = 0;
	POINT mousepos;
	GetCursorPos(&mousepos);
	GetWindowRect(g_RuntimeState.m_hwndBBPager, &r);
	//mousepos.x = mousepos.x - position.x;
	//mousepos.y = mousepos.y - position.y;
	mousepos.x = mousepos.x - r.left;
	mousepos.y = mousepos.y - r.top;

	desktopPressed = -1;

	for (int i = 0; i < g_RuntimeState.m_desktops; i++)
	{
		inDesk = PtInRect(&desktopRect[i], mousepos); // check if mouse cursor is within a desktop RECT
		if (inDesk != 0)
		{
			desktopPressed = i; // if so, set desktop pressed
			return;
		}
	}
}

// This checks the pressed desktop and switches to that, as well as displaying a nice msg :D
void DeskSwitch()
{
	if (desktopPressed > -1 && desktopPressed != g_RuntimeState.m_currentDesktop)
	{
		/*char desktopmsg[MAX_PATH];
		sprintf(desktopmsg, "bbPager -> Change to Workspace %d (%s)", (desktopPressed + 1), desktopName[desktopPressed].c_str());
		SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)desktopmsg);*/
		SendMessage(g_RuntimeState.m_hwndBlackbox, BB_WORKSPACE, 4, (LPARAM)desktopPressed);
	}

	desktopPressed = -1;
}

//===========================================================================
// Focuses window under cursor
void FocusWindow ()
{
	int inWin = 0;
	POINT mousepos;
	GetCursorPos(&mousepos);
	RECT r;
	GetWindowRect(g_RuntimeState.m_hwndBBPager, &r);
	//mousepos.x = mousepos.x - position.x;
	//mousepos.y = mousepos.y - position.y;
	mousepos.x = mousepos.x - r.left;
	mousepos.y = mousepos.y - r.top;

	winPressed = -1;

	for (int i = 0; i <= (g_RuntimeState.m_winCount - 1); i++)
	{
		inWin = PtInRect(&g_RuntimeState.m_winList[i].r, mousepos); // check if mouse cursor is within a window RECT
		if (inWin != 0)
		{
			winPressed = i; // if so, set desktop pressed
			SendMessage(g_RuntimeState.m_hwndBlackbox, BB_BRINGTOFRONT, 0, (LPARAM)g_RuntimeState.m_winList[winPressed].window);

			InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
			return;
		}
	}
}

//===========================================================================
// Picks up window under cursor and drops it wherever
// (according to top left corner of window)

void GrabWindow ()
{
	if (!getSettings().m_desktop.windows)
		return;

	POINT mousepos;
	GetCursorPos(&mousepos);
	RECT r;
	GetWindowRect(g_RuntimeState.m_hwndBBPager, &r);
	mousepos.x -= r.left;
	mousepos.y -= r.top;

	for (int i = (g_RuntimeState.m_winCount - 1); i > -1; i--)
	{
		int const inWin = PtInRect(&g_RuntimeState.m_winList[i].r, mousepos); // check if mouse cursor is within a window RECT
		if (inWin != 0)
		{
			moveWin = g_RuntimeState.m_winList[i];
			grabDesktop = moveWin.desk;
			oldx = moveWin.r.left;
			oldy = moveWin.r.top;
			mx = mousepos.x - moveWin.r.left;
			my = mousepos.y - moveWin.r.top;
			winMoving = true;
			tempTool = getSettings().m_desktop.tooltips;
			getSettings().m_desktop.tooltips = false;
		}
	}
}

void DropWindow ()
{
	Settings const & s = getSettings();
	if (!s.m_desktop.windows)
		return;

	int inDesk = 0, newx, newy;
	POINT pos;

	newx = moveWin.r.left;
	newy = moveWin.r.top;

	if (newx == oldx && newy == oldy)
	{
		SetActiveWindow(lastActive);
		SetForegroundWindow(lastActive);
		return;
	}

	pos.x = moveWin.r.left + ((moveWin.r.right - moveWin.r.left) / 2);
	pos.y = moveWin.r.top + ((moveWin.r.bottom - moveWin.r.top) / 2);

	int dropDesktop = -1;

	for (int i = 0; i < g_RuntimeState.m_desktops; i++)
	{
		inDesk = PtInRect(&desktopRect[i], pos); // check if middle of moving window is within a desktop RECT
		if (inDesk != 0)
		{
			dropDesktop = i; // if so, set desktop pressed

			if (!g_RuntimeState.m_is_xoblite)
			{
				taskinfo ti;
				int stl_flags;

				ti.desk = dropDesktop;
				RECT wRect; GetWindowRect(moveWin.window, &wRect);
				ti.width = wRect.right - wRect.left;
				ti.height = wRect.bottom - wRect.top;

				if (!IsZoomed(moveWin.window))
				{
					int offX = moveWin.r.left - desktopRect[dropDesktop].left;
					int offY = moveWin.r.top - desktopRect[dropDesktop].top;

					int movX = int(offX * s.m_ratioX);
					int movY = int(offY * s.m_ratioY);

					if (movX < 0) movX = 0;
					if (movY < 0) movY = 0;
					if (movX + ti.width > s.m_vScreenWidth) movX = s.m_vScreenWidth - ti.width;
					if (movY + ti.height > s.m_vScreenHeight) movY = s.m_vScreenHeight - ti.height;

					ti.xpos = movX;
					ti.ypos = movY;

					stl_flags = BBTI_SETDESK | BBTI_SETPOS;
				}
				else
				{
					ti.xpos = ti.ypos = 0;

					stl_flags = BBTI_SETDESK;
				}			

				if (moveWin.active || dropDesktop == g_RuntimeState.m_currentDesktop)
					stl_flags |= BBTI_SWITCHTO;

				BBPager_SetTaskLocation(moveWin.window, &ti, stl_flags);
			}
			else
			{
				int diff = dropDesktop - g_RuntimeState.m_currentDesktop;
				int offX = moveWin.r.left - desktopRect[dropDesktop].left;
				int offY = moveWin.r.top - desktopRect[dropDesktop].top;

				int movX = int((diff * (s.m_vScreenWidth + 10) + s.m_vScreenLeft) + (offX * s.m_ratioX));
				int movY = int((offY * s.m_ratioX));

				if (IsZoomed(moveWin.window))
				{
					if (dropDesktop == grabDesktop)
						return;
					if (moveWin.active)
						SetWindowPos(moveWin.window, HWND_TOP, (diff * (s.m_vScreenWidth + 10) - 4 + s.m_vScreenLeft) + s.m_leftMargin, -4 + s.m_topMargin, 300, 300, SWP_NOSIZE);
					else
						SetWindowPos(moveWin.window, HWND_TOP, (diff * (s.m_vScreenWidth + 10) - 4) + s.m_leftMargin, -4 + s.m_topMargin, 300, 300, SWP_NOSIZE | SWP_NOACTIVATE);
				}
				else
				{
					if (moveWin.active)
						SetWindowPos(moveWin.window, HWND_TOP, movX, movY, 300, 300, SWP_NOSIZE);
					else
						SetWindowPos(moveWin.window, HWND_TOP, movX, movY, 300, 300, SWP_NOSIZE | SWP_NOACTIVATE);
				}

				SetTaskWorkspace(moveWin.window, dropDesktop);

				if (g_RuntimeState.m_currentDesktop != dropDesktop && moveWin.active)
					SendMessage(g_RuntimeState.m_hwndBlackbox, BB_WORKSPACE, 4, (LPARAM)dropDesktop);
			}

			char temp[40];
			GetWindowText(moveWin.window, temp, 32);
			strcat(temp, "...");

			if (!GetWindowLong(moveWin.window, WS_VISIBLE) && dropDesktop == g_RuntimeState.m_currentDesktop)
				ShowWindow(moveWin.window, SW_SHOW);

			if (moveWin.active || dropDesktop == g_RuntimeState.m_currentDesktop)
			{
				if (!passive)
				{
				SetActiveWindow(moveWin.window);
				SetForegroundWindow(moveWin.window);
				}
				else
					passive = false;
			}
			else
			{
				SetActiveWindow(lastActive);
				SetForegroundWindow(lastActive);
			}

			return;
		}
	}

	SetActiveWindow(lastActive);
	SetForegroundWindow(lastActive);
	return;
}

//===========================================================================

void TrackMouse()
{
	TRACKMOUSEEVENT tme0;
	tme0.cbSize = sizeof(TRACKMOUSEEVENT);
	tme0.dwFlags = TME_LEAVE;
	tme0.hwndTrack = g_RuntimeState.m_hwndBBPager;
	tme0.dwHoverTime = 0;
	TrackMouseEvent(&tme0);

	TRACKMOUSEEVENT tme1;
	tme1.cbSize = sizeof(TRACKMOUSEEVENT);
	tme1.dwFlags = TME_LEAVE | TME_NONCLIENT;
	tme1.hwndTrack = g_RuntimeState.m_hwndBBPager;
	tme1.dwHoverTime = 0;
	TrackMouseEvent(&tme1);
}

void HidePager()
{
	Settings & s = getSettings();
	if (!inSlit)
	{
		if (!s.m_position.hidden)
		{
			if (CursorOutside())
			{
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				GetPos(true);
				s.m_position.x = s.m_position.hx;
				s.m_position.y = s.m_position.hy;
				MoveWindow(g_RuntimeState.m_hwndBBPager, s.m_position.x, s.m_position.y, s.m_frame.width, s.m_frame.height, true);
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, true);
				s.m_position.hidden = true;
			}		
		}
	}
}

void SetPos(int place)
{
	Settings & s = getSettings();
	//UpdateMonitorInfo();
	switch (place)
	{
		case 3:
		{
			s.m_position.x = s.m_position.ox = s.m_screenLeft;
			s.m_position.y = s.m_position.oy = s.m_screenTop;

			if (s.m_position.vertical)
			{
				s.m_position.hx = s.m_frame.hideWidth - s.m_frame.width + s.m_screenLeft;
				s.m_position.hy = s.m_position.oy;
			}
			else
			{
				s.m_position.hy = s.m_frame.hideWidth - s.m_frame.height + s.m_screenTop;
				s.m_position.hx = s.m_position.ox;
			}
			return;
		}
		break;

		case 6:
		{
			s.m_position.x = s.m_position.ox = s.m_screenWidth - s.m_frame.width + s.m_screenLeft;
			s.m_position.y = s.m_position.oy = s.m_screenTop;

			if (s.m_position.vertical)
			{
				s.m_position.hx = s.m_screenWidth - s.m_frame.hideWidth + s.m_screenLeft;
				s.m_position.hy = s.m_position.oy;
			}
			else
			{
				s.m_position.hy = s.m_frame.hideWidth - s.m_frame.height + s.m_screenTop;
				s.m_position.hx = s.m_position.ox;
			}
			return;
		}
		break;

		case 9:
		{
			s.m_position.y = s.m_position.oy = s.m_screenHeight - s.m_frame.height + s.m_screenTop;
			s.m_position.x = s.m_position.ox = s.m_screenLeft;

			if (s.m_position.vertical)
			{
				s.m_position.hx = s.m_frame.hideWidth - s.m_frame.width + s.m_screenLeft;
				s.m_position.hy = s.m_position.oy;
			}
			else
			{
				s.m_position.hy = s.m_screenHeight - s.m_frame.hideWidth + s.m_screenTop;
				s.m_position.hx = s.m_position.ox;
			}
			return;
		}
		break;

		case 12:
		{		
			s.m_position.y = s.m_position.oy = s.m_screenHeight - s.m_frame.height + s.m_screenTop;
			s.m_position.x = s.m_position.ox = s.m_screenWidth - s.m_frame.width + s.m_screenLeft;

			if (s.m_position.vertical)
			{
				s.m_position.hx = s.m_screenWidth - s.m_frame.hideWidth + s.m_screenLeft;
				s.m_position.hy = s.m_position.oy;
			}
			else
			{
				s.m_position.hy = s.m_screenHeight - s.m_frame.hideWidth + s.m_screenTop;
				s.m_position.hx = s.m_position.ox;
			}
			return;	
		}
		break;

		case 1:
		{
			s.m_position.x = s.m_position.ox = s.m_screenLeft;
			s.m_position.y = s.m_position.oy = (s.m_screenHeight / 2) - (s.m_frame.height / 2) + s.m_screenTop;

			s.m_position.hx = s.m_frame.hideWidth - s.m_frame.width + s.m_screenLeft;
			s.m_position.hy = s.m_position.oy;
			return;
		}
		break;

		case 2:
		{
			s.m_position.y = s.m_position.oy = s.m_screenTop;
			s.m_position.x = s.m_position.ox = (s.m_screenWidth / 2) - (s.m_frame.width / 2) + s.m_screenLeft;

			s.m_position.hy = s.m_frame.hideWidth - s.m_frame.height + s.m_screenTop;
			s.m_position.hx = s.m_position.ox;
			return;
		}
		break;

		case 4:
		{
			s.m_position.x = s.m_position.ox = s.m_screenWidth - s.m_frame.width + s.m_screenLeft;
			s.m_position.y = s.m_position.oy = (s.m_screenHeight / 2) - (s.m_frame.height / 2) + s.m_screenTop;

			s.m_position.hx = s.m_screenWidth - s.m_frame.hideWidth + s.m_screenLeft;
			s.m_position.hy = s.m_position.oy;
			return;
		}
		break;
	
		case 8:
		{
			s.m_position.y = s.m_position.oy = s.m_screenHeight - s.m_frame.height + s.m_screenTop;
			s.m_position.x = s.m_position.ox = (s.m_screenWidth / 2) - (s.m_frame.width / 2) + s.m_screenLeft;

			s.m_position.hy = s.m_screenHeight - s.m_frame.hideWidth + s.m_screenTop;
			s.m_position.hx = s.m_position.ox;
			return;
		}
		break;

		default:
		{
			return;
		}
		break;
	}
}

void GetPos (bool snap)
{
	Settings & s = getSettings();
	if (!s.m_position.hidden)
	{
		s.m_position.ox = s.m_position.x;
		s.m_position.oy = s.m_position.y;

		if (s.m_position.ox == s.m_screenLeft && s.m_position.oy == ((s.m_screenHeight / 2) - (s.m_frame.height / 2)) + s.m_screenTop)
		{
			s.m_position.side = 1; //CL
			return;
		}
		if (s.m_position.ox == s.m_screenLeft && s.m_position.oy == s.m_screenTop)
		{
			s.m_position.side = 3; //TL
			return;
		}
		if (s.m_position.oy == s.m_screenTop && s.m_position.ox == ((s.m_screenWidth / 2) - (s.m_frame.width / 2)) + s.m_screenLeft)
		{
			s.m_position.side = 2; //TC
			return;
		}
		if (s.m_position.ox + s.m_frame.width == s.m_screenRight && s.m_position.oy == s.m_screenTop)
		{
			s.m_position.side = 6; //TR
			return;	
		}
		if (s.m_position.ox == s.m_screenRight - s.m_frame.width && s.m_position.oy == ((s.m_screenHeight / 2) - (s.m_frame.height / 2)) + s.m_screenTop)
		{
			s.m_position.side = 4; //CR
			return;
		}
		if (s.m_position.oy + s.m_frame.height == s.m_screenBottom && s.m_position.ox == s.m_screenLeft)
		{
			s.m_position.side = 9; //BL
			return;
		}
		if (s.m_position.oy + s.m_frame.height == s.m_screenBottom && s.m_position.ox + s.m_frame.width == s.m_screenRight)
		{
			s.m_position.side = 12; //BR
			return;
		}
		if (s.m_position.oy + s.m_frame.height == s.m_screenBottom && s.m_position.ox == ((s.m_screenWidth / 2) - (s.m_frame.width / 2)) + s.m_screenLeft)
		{
			s.m_position.side = 8; //BC
			return;
		}

		s.m_position.side = 0;

		if (snap)
		{
			int left, right, top, bottom;

			left = s.m_position.ox;
			top = s.m_position.oy;
			right = s.m_screenWidth - (s.m_position.ox + s.m_frame.width) + s.m_screenLeft;
			bottom = s.m_screenHeight - (s.m_position.oy + s.m_frame.height) + s.m_screenTop;
			
			if (left <=	top && left <= right && left <= bottom)
			{
				s.m_position.ox = s.m_position.x = s.m_screenLeft;
				s.m_position.oy = s.m_position.y;
				s.m_position.hx = s.m_frame.hideWidth - s.m_frame.width;
				s.m_position.hy = s.m_position.oy;

				if (s.m_position.y == ((s.m_screenHeight / 2) - (s.m_frame.height / 2)) + s.m_screenTop)
					s.m_position.side = 1;

				return;
			}
			if (top <= bottom && top <= left && top <= right)
			{
				s.m_position.ox = s.m_position.x;
				s.m_position.oy = s.m_position.y = s.m_screenTop;
				s.m_position.hy = s.m_frame.hideWidth - s.m_frame.height;
				s.m_position.hx = s.m_position.ox;

				if (s.m_position.x == ((s.m_screenWidth / 2) - (s.m_frame.width / 2)) + s.m_screenLeft)
					s.m_position.side = 2;

				return;
			}
			if (right <= top && right <= bottom && right <= left)
			{
				s.m_position.ox = s.m_position.x = s.m_screenWidth - s.m_frame.width + s.m_screenLeft;
				s.m_position.oy = s.m_position.y;
				s.m_position.hx = s.m_screenWidth - s.m_frame.hideWidth + s.m_screenLeft;
				s.m_position.hy = s.m_position.oy;
				
				if (s.m_position.y == ((s.m_screenHeight / 2) - (s.m_frame.height / 2)) + s.m_screenTop)
					s.m_position.side = 4;

				return;
			}
			if (bottom <= top && bottom <= left && bottom <= right)
			{
				s.m_position.ox = s.m_position.x;
				s.m_position.oy = s.m_position.y = s.m_screenHeight - s.m_frame.height + s.m_screenTop;
				s.m_position.hy = s.m_screenHeight - s.m_frame.hideWidth + s.m_screenTop;
				s.m_position.hx = s.m_position.ox;	
				
				if (s.m_position.x == ((s.m_screenWidth / 2) - (s.m_frame.width / 2)) + s.m_screenLeft)
					s.m_position.side = 8;

				return;
			}
			
			s.m_position.hy = s.m_position.oy = s.m_position.y;
			s.m_position.hx = s.m_position.ox = s.m_position.x;
			s.m_position.hidden = false;
			s.m_position.autohide = false;
			return;
		}
	}
}

//===========================================================================

void UpdatePosition()
{
	Settings & s = getSettings();
	// save old dimensions for slit updating
	heightOld = s.m_frame.height;
	widthOld = s.m_frame.width;
	posXOld = s.m_position.x;
	posYOld = s.m_position.y;

	// =======================
	// Size of pager
	
	g_RuntimeState.m_desktops = 0; // reset number of desktops
	g_RuntimeState.m_desktopsY = 0;
	desktopName.clear();
	SendMessage(g_RuntimeState.m_hwndBlackbox, BB_LISTDESKTOPS, (WPARAM)g_RuntimeState.m_hwndBBPager, 0);
	// currentDesktop gives current desktop number starting at _0_ !

	if (s.m_position.grid)
	{
			s.m_frame.width = (unsigned int)((g_RuntimeState.m_desktops - 1) / s.m_frame.rows + 1) *
				(s.m_desktop.width + s.m_frame.bevelWidth) +
				s.m_frame.bevelWidth + (2 * s.m_frame.borderWidth);

			s.m_frame.height = (unsigned int)(((g_RuntimeState.m_desktops - 1) / s.m_frame.columns) + 1) *
				(s.m_desktop.height + s.m_frame.bevelWidth) +
				s.m_frame.bevelWidth + (2 * s.m_frame.borderWidth);
	}
	else
	{
		// Set window width and height based on number of desktops
		// Takes into account of row/column setting
		if (s.m_position.horizontal) // row/horizontal
		{
			s.m_frame.width = (unsigned int)((g_RuntimeState.m_desktops - 1) / s.m_frame.rows + 1) *
				(s.m_desktop.width + s.m_frame.bevelWidth) +
				s.m_frame.bevelWidth + (2 * s.m_frame.borderWidth);

			if (g_RuntimeState.m_desktops < s.m_frame.rows)
				s.m_frame.height = (unsigned int)(g_RuntimeState.m_desktops * (s.m_desktop.height + s.m_frame.bevelWidth)) + s.m_frame.bevelWidth
				+ (2 * s.m_frame.borderWidth);
			else
				s.m_frame.height = (unsigned int)(s.m_frame.rows * (s.m_desktop.height + s.m_frame.bevelWidth)) + s.m_frame.bevelWidth
				+ (2 * s.m_frame.borderWidth);
		}
		else // column/vertical
		{
			if (g_RuntimeState.m_desktops < s.m_frame.columns)
				s.m_frame.width = (unsigned int)(g_RuntimeState.m_desktops * (s.m_desktop.width + s.m_frame.bevelWidth)) + s.m_frame.bevelWidth
				+ (2 * s.m_frame.borderWidth);
			else
				s.m_frame.width = (unsigned int)(s.m_frame.columns * (s.m_desktop.width + s.m_frame.bevelWidth)) + s.m_frame.bevelWidth
				+ (2 * s.m_frame.borderWidth);

			s.m_frame.height = (unsigned int)(((g_RuntimeState.m_desktops - 1) / s.m_frame.columns) + 1) *
				(s.m_desktop.height + s.m_frame.bevelWidth) +
				s.m_frame.bevelWidth + (2 * s.m_frame.borderWidth);
		}
	}

	// ========================
	// Keep in screen area if no autohide :)

	//UpdateMonitorInfo();
	if (!s.m_position.autohide)
	{
		if (s.m_position.x < s.m_vScreenLeft)
			s.m_position.x = s.m_vScreenLeft;
		if ((s.m_position.x + s.m_frame.width) >(s.m_vScreenWidth + s.m_vScreenLeft))
			s.m_position.x = s.m_vScreenWidth - s.m_frame.width + s.m_vScreenLeft;

		if (s.m_position.y < s.m_vScreenTop)
			s.m_position.y = s.m_vScreenTop;
		if ((s.m_position.y + s.m_frame.height) >(s.m_vScreenHeight + s.m_vScreenTop))
			s.m_position.y = s.m_vScreenHeight - s.m_frame.height + s.m_vScreenTop;
	}
	else
	{
		if (s.m_position.hx <= s.m_vScreenLeft - s.m_frame.width)
			s.m_position.hx = s.m_vScreenLeft - s.m_frame.width + s.m_frame.hideWidth;
		if (s.m_position.hx >= (s.m_vScreenWidth + s.m_vScreenLeft))
			s.m_position.hx = s.m_vScreenWidth - s.m_frame.hideWidth + s.m_vScreenLeft;

		if (s.m_position.hy <= s.m_vScreenTop - s.m_frame.height)
			s.m_position.hy = s.m_vScreenTop - s.m_frame.height + s.m_frame.hideWidth;
		if (s.m_position.hy >= (s.m_vScreenHeight + s.m_vScreenTop))
			s.m_position.hy = s.m_vScreenHeight - s.m_frame.hideWidth + s.m_vScreenTop;
	}

	//===============
	
	if (!s.m_position.hidden)
	{
		s.m_position.ox = s.m_position.x;
		s.m_position.oy = s.m_position.y;
	}

	if (s.m_position.autohide && !inSlit)
	{
		GetPos(true);
		SetPos(s.m_position.side);
		s.m_position.hidden = false;
		HidePager();
		return;
	}

	GetPos(false);
	SetPos(s.m_position.side);

	// Reset BBPager window's position and dimensions
	if (!inSlit)
	{
		if (heightOld != s.m_frame.height || widthOld != s.m_frame.width || posXOld != s.m_position.x || posYOld != s.m_position.y)
			MoveWindow(g_RuntimeState.m_hwndBBPager, s.m_position.x, s.m_position.y, s.m_frame.width, s.m_frame.height, true);
	}
	else if (heightOld != s.m_frame.height || widthOld != s.m_frame.width)
	{
		SetWindowPos(g_RuntimeState.m_hwndBBPager, HWND_TOP, 0, 0, s.m_frame.width, s.m_frame.height, SWP_NOMOVE | SWP_NOZORDER);

		if (hSlit && inSlit)
		{
			SendMessage(hSlit, SLIT_UPDATE, 0, 0);
		}
	}
}

//===========================================================================
//Flashing tasks

void AddFlash(HWND task)
{
	for (int i = 0; i < (int)flashList.size(); i++)
	{
		if (flashList[i].task == task)
		{
			flashList[i].on = !flashList[i].on;
			return;
		}
	}

	FlashTask flashTemp;

	flashTemp.task = task;
	flashTemp.on = true;
	flashList.push_back(flashTemp);
}

void RemoveFlash(HWND task, bool quick)
{
	for (int i = 0; i < (int)flashList.size(); i++)
	{
		if (flashList[i].task == task)
		{
			flashList.erase(flashList.begin() + i);
			if (quick)
				InvalidateRect(g_RuntimeState.m_hwndBBPager, NULL, false);
			return;
		}
	}
}

bool IsFlashOn(HWND task)
{
	for (int i = 0; i < (int)flashList.size(); i++)
	{
		if (flashList[i].task == task && flashList[i].on)
			return true;
	}
	return false;
}

//===========================================================================
//Tooltips

void *m_alloc(unsigned s) { return malloc(s); }
void *c_alloc(unsigned s) { return calloc(1,s); }
void m_free(void *v)	  { free(v); }

struct tt
{
	struct tt *next;
	char used_flg;
	char text[256];
	TOOLINFO ti;
} *tt0;

void SetToolTip(RECT *tipRect, char *tipText)
{
	if (NULL==hToolTips || !*tipText) return;

	struct tt **tp, *t; unsigned n=0;
	for (tp=&tt0; NULL!=(t=*tp); tp=&t->next)
	{
		if (0==memcmp(&t->ti.rect, tipRect, sizeof(RECT)))
		{
			t->used_flg = 1;
			if (0!=strcmp(t->ti.lpszText, tipText))
			{
				strcpy(t->text, tipText);
				SendMessage(hToolTips, TTM_UPDATETIPTEXT, 0, (LPARAM)&t->ti);
			}
			return;
		}
		if (t->ti.uId > n)
			n = t->ti.uId;
	}

	t = (struct tt*)c_alloc(sizeof (*t));
	t->used_flg  = 1;
	t->next = NULL;
	strcpy(t->text, tipText);
	*tp = t;

	memset(&t->ti, 0, sizeof(TOOLINFO));

	t->ti.cbSize   = sizeof(TOOLINFO);
	t->ti.uFlags   = TTF_SUBCLASS;
	t->ti.hwnd = g_RuntimeState.m_hwndBBPager;
	t->ti.uId	   = n+1;
	t->ti.lpszText = t->text;
	t->ti.rect	   = *tipRect;

	SendMessage(hToolTips, TTM_ADDTOOL, 0, (LPARAM)&t->ti);

	HFONT toolFont	= CreateStyleFont((StyleItem *)GetSettingPtr(SN_TOOLBAR));
	SendMessage(hToolTips, WM_SETFONT, (WPARAM)toolFont, (LPARAM)true);
}

//===========================================================================
// Function: ClearToolTips
// Purpose:  clear all tooltips, which are not longer used
//===========================================================================

void ClearToolTips(void)
{
	struct tt **tp, *t;
	tp=&tt0; while (NULL!=(t=*tp))
	{
		if (0==t->used_flg)
		{
			SendMessage(hToolTips, TTM_DELTOOL, 0, (LPARAM)&t->ti);
			*tp=t->next;
			m_free(t);
		}
		else
		{
			t->used_flg = 0;
			tp=&t->next;
		}
	}
}

//===========================================================================

bool BBPager_SetTaskLocation(HWND hwnd, taskinfo const *pti, UINT flags)
{
	if (NULL == BBPager_STL)
	{
		MessageBox(0,"bbPager_SetTaskLocation NO","bbPager_SetTaskLocation NO",MB_OK);
		return false;
	}

	return ( 1 == BBPager_STL(hwnd, pti, flags) );
}

tasklist const * BBPager_GetTaskListPtr()
{
	if (NULL == BBPager_GTLP)
	{
		MessageBox(0,"bbPager_GetTaskListPtr NO","bbPager_GetTaskListPtr NO",MB_OK);
		return NULL;
	}
	
	return BBPager_GTLP();
}

