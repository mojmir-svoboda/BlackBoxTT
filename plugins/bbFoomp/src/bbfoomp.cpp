#include "bbfoomp.h"
#include "settings.h"
#include "styles.h"
#include <tchar.h>
#include <string>
#include <vector>
#include <bblibcompat/bblibcompat.h>
#include <bblibcompat/StyleStruct.h>
#include <bblib/utils_paths.h>
#include <3rd_party/Assert/Assert.h>

#define BBFOOMP_UPDATE_TIMER 1
#define BB_BRINGTOFRONT 10504

#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP 0x020C
#define WM_NCXBUTTONDOWN 0x00AB
#define WM_NCXBUTTONUP 0x00AC

LPSTR szAppName = "bbFoomp";			// The name of our window class, etc.
LPSTR szVersion = "bbFoomp 1.9";		// Used in MessageBox titlebars
LPSTR szInfoVersion = "1.9";
LPSTR szInfoAuthor = "freeb0rn";
LPSTR szInfoRelDate = "2016";
LPSTR szInfoLink = "http://freeb0rn.com";
LPSTR szInfoEmail = "isidoros.passadis@gmail.com";
LPSTR szInfoUpdateURL = "https://blackbox4windows.com";
wchar_t const * szAppNameW = TEXT("bbFoomp");			// The name of our window class, etc.
wchar_t const * szVersionW = TEXT("bbFoomp 1.9");		// Used in MessageBox titlebars
wchar_t const * szInfoVersionW = TEXT("1.9");
wchar_t const * szInfoAuthorW = TEXT("freeb0rn");
wchar_t const * szInfoRelDateW = TEXT("2016");
wchar_t const * szInfoLinkW = TEXT("http://freeb0rn.com");
wchar_t const * szInfoEmailW = TEXT("isidoros.passadis@gmail.com");
wchar_t const * szInfoUpdateURLW = TEXT("https://blackbox4windows.com");

//====================

HINSTANCE hInstance = nullptr;
HWND hwndPlugin = nullptr, hwndBlackbox = nullptr, hwndSlit = nullptr;

// Blackbox messages we want to subscribe to...
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, 0};

int FooModePrev;

// Style items

// Miscellaneous
bool usingWin2kXP = false;
const int button_spacing = 12;
bool FirstUpdate = false; // Moved this up here from below.

// Menu Class
// Menu * scMenu = 0;
// Menu * scSubMenu = 0;
// Menu * scSubMenu2 = 0;

// Display/graphic variables
int DisplayMode;	// If 1 = display mode, then mode = title; if 2 = display mode, then mode = controls.

// Title [DisplayMode] variables, classes, etc.
struct Finfo
{
	wchar_t song_title[MAX_LINE_LENGTH]; // UNICODE
	HWND FooHandle;
	void update ();
};
Finfo * FooClass = 0;
wchar_t CurrentSong[MAX_LINE_LENGTH];
wchar_t DisplayedSong[MAX_LINE_LENGTH];
wchar_t jAmpScrollFiller[256] = L"    ";
bool foobar_v9 = false; // Different commandline syntax needed for newer versions.
bool SlitExists = false;

//===========================================================================

int beginPlugin (HINSTANCE hPluginInstance)
{
	WNDCLASS wc;
	hwndBlackbox = GetBBWnd();
	hInstance = hPluginInstance;

	// Register our window class...
	ZeroMemory(&wc,sizeof(wc));
	wc.lpfnWndProc = WndProc;			// our window procedure
	wc.hInstance = hPluginInstance;		// hInstance of .dll
	wc.lpszClassName = szAppNameW;		// our window class name
	if (!RegisterClass(&wc))
	{
		MessageBox(hwndBlackbox, TEXT("Error registering window class"), szVersionW, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Get plugin and style settings...
	getSettings().ReadRCSettings();
	getStyles().GetStyleSettings();

	if (getSettings().FooDockedToSlit && !hwndSlit)
	{
		MessageBox(0, TEXT("bbFoomp wants to be placed in slit!\nModify your plugins.rc, please."), szVersionW, MB_OK | MB_ICONINFORMATION);
		return 1;
	}
	
	// Tap into the FooClass! (It's variables are going to be used
	// throughout the plugins. Window, handle etc.)
	if (FooClass)
		delete FooClass;	//Make sure FooClass is deleted from previous instances.
	FooClass = new Finfo;

	// Create our window...
	hwndPlugin = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						szAppNameW,										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,	// window parameters
						getSettings().xpos,								// x position
						getSettings().ypos,								// y position
						getSettings().width,							// window width
						getSettings().height,							// window height
						NULL,											// parent window
						NULL,											// no menu
						hPluginInstance,								// hInstance of .dll
						NULL);
	if (!hwndPlugin)
	{
		MessageBox(0, TEXT("Error creating window"), szVersionW, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndPlugin, (LPARAM)msgs);
	// Set window to accept doubleclicks...
	SetClassLong(hwndPlugin, GCL_STYLE, CS_DBLCLKS | GetClassLong(hwndPlugin, GCL_STYLE));
	// Make the plugin window sticky (= pin the window)...
	MakeSticky(hwndPlugin);

	// Should we dock to the slit?
	// *** Thanks to qwilk for all things slit! ***
	if (SlitExists && getSettings().FooDockedToSlit)
		SendMessage(hwndSlit, SLIT_ADD, NULL, (LPARAM)hwndPlugin);

	// Set the update timer to 33 milliseconds...
	const int UpdateInterval = 33;
	if (!SetTimer(hwndPlugin, BBFOOMP_UPDATE_TIMER, UpdateInterval, (TIMERPROC)NULL))
	{
		MessageBox(0, TEXT("Error creating update timer"), szVersionW, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Show our window and force update...
	FirstUpdate = true;
	UpdateTitle();
	ShowWindow(hwndPlugin, SW_SHOW);

	// These are the buttons: reverse, pause, play, stop, forward, playlist, open/add, volul, voldown
	// Here we set them to false, and if OnTop is set in the settings we enable it.
	for (int i = 0; i < e_last_button_item; ++i)
	getSettings().buttons[i].pressed = false;
	if (getSettings().FooOnTop==true)
	{
		SetWindowPos(hwndPlugin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)L"BBFoomp -> Always On Top enabled");
	}
	// This is a check to see if transparency exists in the settings and if so enable it.
	Transparency();
	return 0;
}

//====================

int beginPluginEx (HINSTANCE hPluginInstance, HWND hwndBBSlit)
{
	return beginSlitPlugin(hPluginInstance, hwndBBSlit);
}

int beginSlitPlugin (HINSTANCE hPluginInstance, HWND hwndBBSlit)
{
	SlitExists = true;
	hwndSlit = hwndBBSlit;
	return beginPlugin(hPluginInstance);
}

void endPlugin (HINSTANCE hPluginInstance)
{
	if (!hwndSlit)
		return;

	getSettings().WriteRCSettings();

	if (hwndSlit)
	{
		// Remove from slit...
		SendMessage(hwndSlit, SLIT_REMOVE, NULL, (LPARAM)hwndPlugin);
		// Kill update timer...
		KillTimer(hwndPlugin, BBFOOMP_UPDATE_TIMER);
		// Delete the main plugin menu if it exists (PLEASE NOTE: This takes care of submenus as well!)
// 		if (scMenu) 
// 			DelMenu(scMenu);
		// Unregister Blackbox messages...
		SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndPlugin, (LPARAM)msgs);
		// Destroy our window...
		DestroyWindow(hwndPlugin);
		// Unregister window class...
		UnregisterClass(szAppNameW, hPluginInstance);
		// Delete used FooInfo...
		if (FooClass) delete FooClass;
	}
}

//===========================================================================

LPCSTR pluginInfo (int field)
{
	// pluginInfo is used by Blackbox for Windows to fetch information about
	// a particular plugin. At the moment this information is simply displayed
	// in an "About loaded plugins" MessageBox, but later on this will be
	// expanded into a more advanced plugin handling system. Stay tuned! :)

	switch (field)
	{
		case 1: return szAppName;		// Plugin name
		case 2: return szInfoVersion;	// Plugin version
		case 3: return szInfoAuthor;	// Author
		case 4: return szInfoRelDate;	// Release date, preferably in yyyy-mm-dd format
		case 5: return szInfoLink;		// Link to author's website
		case 6: return szInfoEmail;		// Author's email
// 		case PLUGIN_BROAMS:				// List of bro@ms available to the end users
// 		{
// 			return
// 			"@bbfoomp About "
// 			"@bbfoomp Play_Pause"
// 			"@bbfoomp Stop"
// 			"@bbfoomp Next"
// 			"@bbfoomp Previous"
// 			"@bbfoomp Random"
// 			"@bbfoomp VolUp"
// 			"@bbfoomp VolDown";
// 		}
//		case PLUGIN_UPDATE_URL: return szInfoUpdateURL;		// AutoUpdate URL
		default: return szVersion;		// Fallback: Plugin name + version, e.g. "MyPlugin 1.0"
	}
}

void show_foomp_menu ()
{
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
// 	if (scMenu) DelMenu(scMenu);
// 
// 	scMenu = MakeMenu("bbFoomp");
// 
// 	//	<<	BEGIN	-- Controls Submenu
// 	scSubMenu = MakeMenu("Controls");
// 	MakeMenuItem(scSubMenu, "Previous", "@bbfoomp Previous", false);
// 	MakeMenuItem(scSubMenu, "Play/Pause", "@bbfoomp Play_Pause", false);
// 	MakeMenuItem(scSubMenu, "Stop", "@bbfoomp Stop", false);
// 	MakeMenuItem(scSubMenu, "Next", "@bbfoomp Next", false);
// 	MakeMenuItem(scSubMenu, "Random", "@bbfoomp Random", false);
// 	MakeMenuItem(scSubMenu, "Open file", "@bbfoomp Open", false);
// 	MakeMenuItem(scSubMenu, "Add files", "@bbfoomp Add", false);
// 	if (FooClass->FooHandle)
// 	{
// 		MakeMenuNOP(scSubMenu, ""); // Separator
// 		MakeMenuItem(scSubMenu, "Foobar > Off", "@bbfoomp FooOff", false);
// 	}
// 	MakeSubmenu(scMenu, scSubMenu, "Controls");
// 	//	>>	END		-- Controls Submenu
// 
// 	//	<<	BEGIN	-- Playback Order Submenu
// 	scSubMenu = MakeMenu("Playback Order");
// 	MakeMenuItem(scSubMenu, "Default", "@bbfoomp Order_Default", false);
// 	MakeMenuItem(scSubMenu, "Random", "@bbfoomp Order_Random", false);
// 	MakeMenuItem(scSubMenu, "Repeat", "@bbfoomp Order_Repeat", false);
// 	MakeMenuItem(scSubMenu, "Repeat One", "@bbfoomp Order_RepeatOne", false);
// 	MakeSubmenu(scMenu, scSubMenu, "Playback Order");
// 	//	>>	END		-- Playback Order Submenu
// 	
// 	//	<<	BEGIN	-- Options Submenu
// 	scSubMenu = MakeMenu("Options");
// 	if (getSettings().FooMode != 3)MakeMenuItem(scSubMenu, "Toggle Display Mode", "@bbfoomp ToggDispMode", false);
// 	if (getSettings().FooMode != 3) MakeMenuItem(scSubMenu, "FooMouseOver Mode", "@bbfoomp ToggFooMode", (getSettings().FooMode == 1));
// 	MakeMenuItem(scSubMenu, "FooMega Mode", "@bbfoomp ToggFooMega", (getSettings().FooMode == 3));
// 	MakeMenuNOP(scSubMenu, ""); // Septarator
// 	MakeMenuItem(scSubMenu, "Edit RC Settings", "@bbfoomp EditSettings", false);
// 	MakeMenuItem(scSubMenu, "Read RC Settings", "@bbfoomp ReadSettings", false);
// 	MakeSubmenu(scMenu, scSubMenu, "Options");
// 	//	>>	END		-- Options Submenu
// 
// 	MakeMenuNOP(scMenu, "");// Separator
// 
// 	//	<<	BEGIN	-- Preferences Submenu
// 	scSubMenu = MakeMenu("Preferences");
// 		scSubMenu2 = MakeMenu("Inset Rectangle Style");
// 			MakeMenuItem(scSubMenu2, "Label", "@bbfoomp ChangeInnerStyle 1", (getSettings().InnerStyleIndex == 1));
// 			MakeMenuItem(scSubMenu2, "Window Label", "@bbfoomp ChangeInnerStyle 2", (getSettings().InnerStyleIndex == 2));
// 			MakeMenuItem(scSubMenu2, "Clock", "@bbfoomp ChangeInnerStyle 3", (getSettings().InnerStyleIndex == 3));
// 			MakeMenuItem(scSubMenu2, "Toolbar", "@bbfoomp ChangeInnerStyle 4", (getSettings().InnerStyleIndex == 4));
// 			MakeMenuItem(scSubMenu2, "Button", "@bbfoomp ChangeInnerStyle 5", (getSettings().InnerStyleIndex == 5));
// 			MakeMenuItem(scSubMenu2, "Button.Pressed", "@bbfoomp ChangeInnerStyle 6", (getSettings().InnerStyleIndex == 6));
// 			MakeSubmenu(scSubMenu, scSubMenu2, "Inset Rectangle Style");
// 		scSubMenu2 = MakeMenu("Outer Rectangle Style");
// 			MakeMenuItem(scSubMenu2, "Label", "@bbfoomp ChangeOuterStyle 1", (getSettings().OuterStyleIndex == 1));
// 			MakeMenuItem(scSubMenu2, "Window Label", "@bbfoomp ChangeOuterStyle 2", (getSettings().OuterStyleIndex == 2));
// 			MakeMenuItem(scSubMenu2, "Clock", "@bbfoomp ChangeOuterStyle 3", (getSettings().OuterStyleIndex == 3));
// 			MakeMenuItem(scSubMenu2, "Toolbar", "@bbfoomp ChangeOuterStyle 4", (getSettings().OuterStyleIndex == 4));
// 			MakeMenuItem(scSubMenu2, "Button", "@bbfoomp ChangeOuterStyle 5", (getSettings().OuterStyleIndex == 5));
// 			MakeMenuItem(scSubMenu2, "Button.Pressed", "@bbfoomp ChangeOuterStyle 6", (getSettings().OuterStyleIndex == 6));
// 			MakeSubmenu(scSubMenu, scSubMenu2, "Outer Rectangle Style");
// 		scSubMenu2 = MakeMenu("FooMegaMode Alignment");
// 			MakeMenuItem(scSubMenu2, "Buttons on Right", "@bbfoomp ChangeMegaAlign 1", (getSettings().FooAlign == false));
// 			MakeMenuItem(scSubMenu2, "Buttons on Left", "@bbfoomp ChangeMegaAlign 2", (getSettings().FooAlign == true));
// 		MakeSubmenu(scSubMenu, scSubMenu2, "FooMegaMode Alignment");
// 	MakeMenuItem(scSubMenu, "Shadows enabled", "@bbfoomp ToggleShadows", getSettings().FooShadowsEnabled);
// 	MakeMenuItemInt(scSubMenu,"Scroll speed", "@bbfoomp ScrollSpeed", getSettings().FooScrollSpeed, 1,10);
// 	if (SlitExists) MakeMenuItem(scSubMenu, "Docked to slit", "@bbfoomp ToggleDockedToSlit", getSettings().FooDockedToSlit);
// 	if (!getSettings().FooDockedToSlit) MakeMenuItem(scSubMenu, "Always on top", "@bbfoomp ToggleOnTop", getSettings().FooOnTop);
// 	if (!getSettings().FooDockedToSlit) MakeMenuItem(scSubMenu, "Transparency", "@bbfoomp ToggleTrans", getSettings().FooTrans);
// 	MakeSubmenu(scMenu, scSubMenu, "Preferences");
// 	//	>>	END		-- Preferences Submenu
// 	MakeMenuItem(scMenu, "Readme", "@bbfoomp Readme", false);
// 	MakeMenuItem(scMenu, "About bbFoomp", "@bbfoomp About", false);
// 	ShowMenu(scMenu);
}

void handleBroamMsg (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	wchar_t temp[MAX_LINE_LENGTH];
	wcsncpy(temp, (LPCWSTR)lParam, MAX_LINE_LENGTH);

	// Global bro@ms...
	if (!getSettings().FooDockedToSlit)
	{
		if (!_wcsicmp(temp, L"@BBShowPlugins"))
		{
			// Show window and force update...
			ShowWindow(hwndPlugin, SW_SHOW);
			InvalidateRect(hwndPlugin, NULL, true);
			return;
		}
		else if (!_wcsicmp(temp, L"@BBHidePlugins"))
		{
			// Hide window...
			ShowWindow(hwndPlugin, SW_HIDE);
			return;
		}
	}

	// bbfoomp bro@ms...
	if (wcsstr(temp, L"@bbfoomp"))
	{
		// bbFoomp internal commands (e.g. for the menu)...
		static char msg[MAX_LINE_LENGTH];
		wchar_t token1[4096], token2[4096], extra[4096];
		wchar_t * tokens[2] = { token1, token2 };
		size_t sizes[2] = { 4096, 4096 };

		token1[0] = token2[0] = extra[0] = '\0';
		BBTokenize(temp, tokens, sizes, 2, extra, 4096, true);

		if (!wcsicmp(token2, L"Readme"))
		{
			TCHAR path[MAX_LINE_LENGTH], directory[MAX_LINE_LENGTH];
			// First we look for the readme file in the same folder as the plugin...
			GetModuleFileName(hInstance, path, sizeof(path));
			int nLen = _tcslen(path) - 1;
			while (nLen > 0 && path[nLen] != '\\')
				nLen--;
			path[nLen + 1] = 0;
			_tcscpy(directory, path);
			_tcscat(path, TEXT("bbFoomp.htm"));
			if (bb::fileExists(path))
				BBExecute(GetDesktopWindow(), NULL, path, L"", directory, SW_SHOWNORMAL, true);
		}
		else if (!wcsicmp(token2, L"About"))
		{
			TCHAR temp[MAX_LINE_LENGTH];
			_tcscpy(temp, szVersionW);
			_tcscat(temp, TEXT("\n\n© 2005 isidoros.passadis@gmail.com\n\nhttp://freeb0rn.com/\n#bb4win on irc.freenode.net	"));
			MessageBox(0, temp, TEXT("About this plugin..."), MB_OK | MB_ICONINFORMATION);
			return;
		}

		if (!wcsicmp(token2, L"EditSettings"))
		{
// 			wchar_t temp[MAX_LINE_LENGTH];
// 			GetBlackboxEditor(temp);
// 			if (FileExists(getSettings().rcpath))
// 				BBExecute(GetDesktopWindow(), NULL, temp, getSettings().rcpath.c_str(), NULL, SW_SHOWNORMAL, true);
		}

		if (!wcsicmp(token2, L"ReadSettings"))
		{
			getSettings().ReadRCSettings();
			UpdatePosition(); // Get new settings and resize window if needed...
			SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
			InvalidateRect(hwndPlugin, NULL, false);
		}
		else if (!wcsicmp(token2, L"Show_Hide"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Activate or hide\"" : L"/command:\"foobar2000/Activate or hide\"", NULL, SW_SHOWNORMAL, false);
			SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"ToggDispMode"))
		{
			if (DisplayMode == 1)
				DisplayMode = 2;
			else
				DisplayMode = 1;

			UpdatePosition(); // Get new settings and resize window if needed...
			SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
			InvalidateRect(hwndPlugin, NULL, false);
			return;
		}
		else if (!wcsicmp(token2, L"ChangeInnerStyle"))
		{
			int const val = _wtoi(extra);
			if (val > 0 && val <= 6)
			{
				WriteInt(getSettings().rcpath.c_str(), L"bbfoomp.InnerStyle:", val);
				getSettings().ReadRCSettings();
				UpdatePosition(); // Get new settings and resize window if needed...
				SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
				InvalidateRect(hwndPlugin, NULL, false);
			}
			return;
		}
		else if (!wcsicmp(token2, L"ChangeOuterStyle"))
		{
			int const val = _wtoi(extra);
			if (val > 0 && val <= 6)
			{
				WriteInt(getSettings().rcpath.c_str(), L"bbfoomp.OuterStyle:", val);
				getSettings().ReadRCSettings();
				UpdatePosition(); // Get new settings and resize window if needed...
				SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
				InvalidateRect(hwndPlugin, NULL, false);
			}
			return;
		}
		else if (!wcsicmp(token2, L"ChangeMegaAlign"))
		{
			if (!wcsicmp(extra, L"1"))
			{
				getSettings().FooAlign = false;
				WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.MegaLeftAlign:", getSettings().FooAlign);
			}
			else if (!wcsicmp(extra, L"2"))
			{
				getSettings().FooAlign = true;
				WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.MegaLeftAlign:", getSettings().FooAlign);
			}
			getSettings().ReadRCSettings();
			UpdatePosition(); // Get new settings and resize window if needed...
			SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
			InvalidateRect(hwndPlugin, NULL, false);
			return;
		}
		else if (!wcsicmp(token2, L"ToggFooMode"))
		{
			if (getSettings().FooMode == 1)
			{
				getSettings().FooMode = 2;
				UpdateTitle();
			}
			else
			{
				getSettings().FooMode = 1;
				UpdateTitle();
			}

			UpdatePosition(); // Get new settings and resize window if needed...
			SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
			InvalidateRect(hwndPlugin, NULL, false);
			return;
		}
		else if (!wcsicmp(token2, L"ToggFooMega"))
		{
			if (getSettings().width < 300)
				MessageBox(0, TEXT("Please assign a Width greater than 300\nin the bbfoomp.rc for this feature to work."), L"ERROR: Feature Unusable at this Width", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
			else
			{
				if (getSettings().FooMode != 3)
				{
					FooModePrev = getSettings().FooMode;
				}
				if (getSettings().FooMode == 1 || getSettings().FooMode == 2)
				{
					getSettings().FooMode = 3;
					UpdateTitle();
				}
				else
				{
					getSettings().FooMode = FooModePrev;
					if (getSettings().FooMode == 0) getSettings().FooMode = 1;
					if (getSettings().FooMode == 2) DisplayMode = 1;
					UpdateTitle();
				}
				UpdatePosition(); // Get new settings and resize window if needed...
				SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
				InvalidateRect(hwndPlugin, NULL, false);
			}
			return;
		}
		else if (!wcsicmp(token2, L"ToggleDockedToSlit"))
		{
			ToggleDockedToSlit();
			return;
		}
		else if (!wcsicmp(token2, L"ToggleShadows"))
		{
			getSettings().FooShadowsEnabled = !getSettings().FooShadowsEnabled;
			WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.Shadows:", getSettings().FooShadowsEnabled);
			return;
		}
		else if (!wcsicmp(token2, L"ScrollSpeed"))
		{
			int val = _wtoi(extra);
			if (val > 0 && val <= 10)
			{
				getSettings().FooScrollSpeed = val;
				WriteInt(getSettings().rcpath.c_str(), L"bbfoomp.ScrollSpeed:", getSettings().FooScrollSpeed);
			}
			return;
		}
		else if (!wcsicmp(token2, L"ToggleOnTop"))
		{
			if (getSettings().FooOnTop == true)
			{
				getSettings().FooOnTop = false;
				SetWindowPos(hwndPlugin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)L"BBFoomp -> Always On Top disabled");
				WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.OnTop:", getSettings().FooOnTop);
			}
			else
			{
				getSettings().FooOnTop = true;
				SetWindowPos(hwndPlugin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)L"BBFoomp -> Always On Top enabled");
				WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.OnTop:", getSettings().FooOnTop);
			}
			return;
		}
		else if (!wcsicmp(token2, L"ToggleTrans"))
		{
			if (usingWin2kXP)
			{
				if (getSettings().FooTrans)
				{
					getSettings().FooTrans = false;
					SetTransparency(hwndPlugin, 255);
					SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)L"BBFoomp -> Transparency disabled");
					WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.transparency:", getSettings().FooTrans);
				}
				else if (!getSettings().FooDockedToSlit)
				{
					getSettings().FooTrans = true;
					SetTransparency(hwndPlugin, (unsigned char)getSettings().transparencyAlpha);
					SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)L"BBFoomp -> Transparency enabled");
					WriteBool(getSettings().rcpath.c_str(), L"bbfoomp.transparency:", getSettings().FooTrans);
				}
			}
		}
		// ========== BEGIN CONTROLS BROAMS
		// You will note a few commented lines under each broam,
		// should you wish to uncomment them they will focus Foobar2000
		// whenever a command is given to it.
		else if (!wcsicmp(token2, L"VolUp"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Volume up\"" : L"/command:\"Playback/Volume up\"", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"VolDown"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Volume down\"" : L"/command:\"Playback/Volume down\"", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Play_Pause"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/playpause", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Play"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/play", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Stop"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/stop", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Previous"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/prev", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Next"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/next", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Random"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/rand", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Add"))
		{
			SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)L"@bbfoomp Show_Hide");
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Add files...\"" : L"/command:\"Playlist/Add files...\"", NULL, SW_SHOWNORMAL, false);
			return;
		}
		else if (!wcsicmp(token2, L"Open"))
		{
			SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)L"@bbfoomp Show_Hide");
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Open...\"" : L"/command:\"Playlist/Open...\"", NULL, SW_SHOWNORMAL, false);
			return;
		}
		else if (!wcsicmp(token2, L"FooOff"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), L"/exit", NULL, SW_SHOWNORMAL, false);
			return;
		}
		// ========== END CONTROLS BROAMS // BEGIN PLAYBACK ORDER BROAMS
		else if (!wcsicmp(token2, L"Order_Default"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Default\"" : L"/command:\"Playback/Order/Default\"", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Order_Random"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Shuffle (tracks)\"" : L"/command:\"Playback/Order/Random\"", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Order_Repeat"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Repeat (playlist)\"" : L"/command:\"Playback/Order/Repeat\"", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		else if (!wcsicmp(token2, L"Order_RepeatOne"))
		{
			BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), foobar_v9 ? L"/command:\"Repeat (track)\"" : L"/command:\"Playback/Order/Repeat One\"", NULL, SW_SHOWNORMAL, false);
			//SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
			//SetForegroundWindow(FooClass->FooHandle);
			return;
		}
		// ========== CUSTOM COMMAND BROAMS
		else if (!wcsnicmp(token2, L"Press", 5))
		{
			int const button_idx = _wtoi(token2 + 5);
			if (button_idx > 0 && button_idx < e_last_button_item && getSettings().buttons[button_idx - 1].cmdarg[0])
				BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), getSettings().buttons[button_idx - 1].cmdarg.c_str(), NULL, SW_SHOWNORMAL, false);
			return;
		}
		else if (!wcsnicmp(token2, L"AltPress", 8))
		{
			int button_idx = _wtoi(token2 + 8);
			if (button_idx > 0 && button_idx < e_last_button_item && getSettings().buttons[button_idx - 1].altcmdarg[0])
			{
				BBExecute(GetDesktopWindow(), NULL, getSettings().FooPath.c_str(), getSettings().buttons[button_idx - 1].altcmdarg.c_str(), NULL, SW_SHOWNORMAL, false);
			}
			return;
		}
	}
}

//===========================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC buf = CreateCompatibleDC(NULL);
			HDC src = CreateCompatibleDC(NULL);
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc, getSettings().width, getSettings().height);
			HBITMAP srcbmp = CreateCompatibleBitmap(hdc, 20, 15);
			HBITMAP oldbuf = (HBITMAP)SelectObject(buf, bufbmp);
			RECT r;

			// Paint border+background according to the current style...
			GetClientRect(hwnd, &r);
			MakeStyleGradient(buf, &r, &getStyles().OuterStyle, getStyles().OuterStyle.bordered);

			const int offset = getStyles().OuterStyle.borderWidth + getSettings().BorderWidth;
			r.left += offset;
			r.top += offset;
			r.right -= offset;
			r.bottom -= offset;

			//******* Begin control button calculations
			// Paint pressed button-control buttons...
			if (DisplayMode == 2)
			{
				CalculateButtonPositions(r); //Now we know how where the buttons are in this rect.
				DispModeControls(r, buf);
			}
			// Here follows the DisplayMode = Title stuff. (Scrolling title, etc.)
			if (!DisplayMode)
				DisplayMode = 1;

			if (DisplayMode == 1 || DisplayMode == 3)
			{
				// DisplayMode 3 (side-by-side support)
				if (getSettings().FooMode == 3)
				{
					if (getSettings().FooAlign == false) r.left = getSettings().FooWidth;

					DisplayMode = 3;
					r.right = r.left + 140 + 18;
					DispModeControls(r, buf);
					const int offset3 = 150 + 18;
					r.left = r.left + offset3;
					r.right = getSettings().width - 4;

					if (getSettings().FooAlign == false)
					{
						r.left = getSettings().BorderWidth + 1;
						r.right = getSettings().FooWidth - offset3;
					}
				}

				MakeStyleGradient(buf, &r, &getStyles().InnerStyle, false);
				int offset2 = getStyles().OuterStyle.borderWidth + 3;
				r.left += offset2;
				r.top += offset2;
				r.right -= offset2;
				r.bottom -= offset2;

				//====================
				FooClass->update(); // make sure the current data is updated
				wcscpy_s(CurrentSong, FooClass->song_title);
				// ===== End of grabbing name and handle, time to draw the text.

				// Song title scrolling if songtitle > X characters as it will be clipped.
				if (getSettings().FooMode == 3)
				{
					if (getSettings().FooAlign == true) r.left = r.left + 150 + 18;
					if (getSettings().FooAlign == false) r.left = r.left;
				}
				static int txtRefX = r.left;
				wchar_t temp[512] = L"";
				int textWidth;
				int ret;
				textWidth = r.right;
				r.right = r.right + 2;
				RECT r2;
				SetRectEmpty(&r2);
				r.top = r.top + 1;
				r.top = getSettings().height / 2 - 5;
				r.bottom = r.bottom + 2;

				wcscat(temp, FooClass->song_title); //UNICODE
				wcscat(temp, jAmpScrollFiller); //UNICODE

				HFONT font =  CreateStyleFont((StyleItem *)GetSettingPtr(SN_TOOLBAR));
				HGDIOBJ oldfont = SelectObject(buf, font);
				SetBkMode(buf, TRANSPARENT);
				StyleItem * lbl = (StyleItem *)GetSettingPtr(SN_TOOLBARLABEL);
				ret = BBDrawTextAltW(buf, temp, wcslen(temp), &r2, DT_SINGLELINE | DT_CALCRECT, lbl);

				// Scroll the text if needed
				if ( (r2.right - r2.left + 118) > textWidth && wcslen(FooClass->song_title) > 30)
				{
					SetTextAlign(buf, 0);
					// Title text repeater...
					wcscat_s(temp, FooClass->song_title);
					wcscat_s(temp, jAmpScrollFiller);

					// The actual scroller and the sliding scroll pointer!
					if (DisplayMode == 1 && txtRefX <= (r2.left - (r2.right - 10))) txtRefX = r.left;
					if (getSettings().FooMode == 3 && txtRefX <= (r2.left - (r2.right - 150))) txtRefX = r.left;
					txtRefX -= getSettings().FooScrollSpeed;  // Scroll speed.
					r.left += 2;
					r.right -= 2; //NOTE: I added this
					r.bottom = r.bottom + 1;

					if (getSettings().FooShadowsEnabled)
					{
						RECT srect;
						srect.bottom = r.bottom + 1;
						srect.left   = r.left;
						srect.right  = r.right; // No weird shadow artifacting on the right hand side.
						srect.top    = r.top + 1;
						SetTextColor(buf, GetShadowColor(getStyles().InnerStyle));
						ExtTextOutW(buf, (txtRefX + 1), (srect.top), ETO_CLIPPED, &srect, temp, wcslen(temp), NULL);
					}
					SetTextColor(buf, getStyles().InnerStyle.TextColor);
					ExtTextOutW(buf, (txtRefX), (r.top), ETO_CLIPPED, &r, temp, wcslen(temp), NULL);
				}
				else // Normally draw the text since it doesn't need to scroll/get clipped.
				{
					r.top = getSettings().height / 2 - 5;
					r.left = r.left + 1;

					if (getSettings().FooShadowsEnabled)
					{
						RECT srect;
						srect.bottom = r.bottom + 1;
						srect.left   = r.left + 1;
						srect.right  = r.right + 1;
						srect.top    = r.top + 1;
						SetTextColor(buf, GetShadowColor(getStyles().InnerStyle));
						StyleItem * lbl = (StyleItem *)GetSettingPtr(SN_TOOLBARLABEL);

						BBDrawTextAltW(buf, FooClass->song_title, wcslen(FooClass->song_title), &srect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX, lbl);
					}
					SetTextColor(buf, getStyles().InnerStyle.TextColor);
					BBDrawTextAltW(buf, FooClass->song_title, wcslen(FooClass->song_title), &r, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX, lbl);
				}

				DeleteObject(SelectObject(buf, oldfont));
			}

			// Copy from the paint buffer to the window...
			BitBlt(hdc, 0, 0, getSettings().width, getSettings().height, buf, 0, 0, SRCCOPY);

			DeleteDC(src);
			SelectObject(buf, oldbuf);
			DeleteDC(buf);
			DeleteObject(bufbmp);
			DeleteObject(srcbmp);
			DeleteObject(oldbuf);
			EndPaint(hwnd, &ps);
			return 0;
		}
		// ==========
		case WM_CLOSE:
			return 0;
		// ==========
		case BB_RECONFIGURE:
		{
			UpdatePosition(); // Get new settings and resize window if needed...
			if (getSettings().FooDockedToSlit)
			  SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
			InvalidateRect(hwndPlugin, NULL, false);
			break;
		}
		// ==========
		case WM_TIMER:
		{
			UpdateTitle();
			return 0;
		}
		// ==========
		case BB_BROADCAST:
		{
			handleBroamMsg(hwnd, message, wParam, lParam);
			return 0;
		}
		case WM_NCHITTEST:
		{
			if (!getSettings().FooDockedToSlit && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
				return HTCAPTION;
			else
				return HTCLIENT;
		}
		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP:
			show_foomp_menu();
			break;
		case WM_RBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			break;
		case WM_LBUTTONDBLCLK:
		{
			if (DisplayMode == 1 || DisplayMode == 3)
				SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)L"@bbfoomp Show_Hide");
			break;
		}
		case WM_LBUTTONDOWN:
		{
			TrackMouse();
			ClickMouse(LOWORD(lParam), HIWORD(lParam));
			break;
		}
		case WM_LBUTTONUP:
		{
			for (int i = 0; i < e_last_button_item; ++i)
				if (getSettings().buttons[i].pressed)
				{
					wchar_t buffer[128];
					if (GetAsyncKeyState(VK_MENU) & 0x8000)
						wsprintf(buffer, L"@bbfoomp AltPress%d", i + 1);
					else
						wsprintf(buffer, L"@bbfoomp Press%d", i + 1);
					SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)buffer);
				}
			
			for (int i = 0; i < e_last_button_item; ++i)
				getSettings().buttons[i].pressed = false;
			InvalidateRect(hwndPlugin, NULL, false);
			break;
		}

		case WM_XBUTTONUP:
		case WM_NCXBUTTONUP:
			if (SlitExists)
				ToggleDockedToSlit();
			break;

		case WM_XBUTTONDOWN:
		case WM_NCXBUTTONDOWN:
			break;

		case WM_MOUSELEAVE:
		{
			if (DisplayMode == 2 || DisplayMode == 3)
			{
				for (int i = 0; i < e_last_button_item; ++i)
					getSettings().buttons[i].pressed = false;
				InvalidateRect(hwndPlugin, NULL, false);
				if (getSettings().FooMode == 1) // If 'mouseover' mode is on...
				{
					DisplayMode = 1;
					UpdateTitle();
				}
			}
			break;
		}

		case WM_MOUSEMOVE:
		{
			TrackMouse();
			if (getSettings().FooMode == 1) // If 'mouseover' mode is on...
			{
				if (DisplayMode == 1)
				{
					DisplayMode = 2;
					UpdateTitle();
				}
			}
			break;
		}

		// Snap window to screen edges (or the currently defined DesktopArea)...
		case WM_WINDOWPOSCHANGING:
		{
			if (!getSettings().FooDockedToSlit)
				if (IsWindowVisible(hwnd))
					SnapWindowToEdge((WINDOWPOS*)lParam, 10, true);
			break;
		}

		// Save window position if it changes...
		case WM_WINDOWPOSCHANGED:
		{
			if (!getSettings().FooDockedToSlit)
			{
				WINDOWPOS* windowpos = (WINDOWPOS*)lParam;
				getSettings().xpos = windowpos->x;
				getSettings().ypos = windowpos->y;
			}
			break;
		}

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

void UpdateTitle ()
{
	bool UpdateDisplay = false;

	if (FirstUpdate)
	{
		UpdateDisplay = true;
		FirstUpdate = false;
	}

	// Only update toolbar if the windowname has changed.
	if (CurrentSong != DisplayedSong || UpdateDisplay)
	{
		// Setting the CurrentSong as the Displayed song for the next Update.
		wcscpy(DisplayedSong, CurrentSong);
		UpdateDisplay = true;
	}

	if (UpdateDisplay)
		InvalidateRect(hwndPlugin, NULL, false);
}

void UpdatePosition ()
{
	getStyles().GetStyleSettings();
	UpdateTitle();
	MoveWindow(hwndPlugin, getSettings().xpos, getSettings().ypos, getSettings().width, getSettings().height, true);
	SetWindowPos(hwndPlugin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
}

void ToggleDockedToSlit ()
{
// 	if (getSettings().FooDockedToSlit)
// 	{
// 		SendMessage(hwndSlit, SLIT_REMOVE, NULL, (LPARAM)hwndPlugin);
// 		getSettings().FooDockedToSlit = false;
// 		// Since position in slit != position as plugin we need to reload the position...
// 		getSettings().xpos = ReadInt(getSettings().rcpath, "bbfoomp.xpos:", 0);
// 		getSettings().ypos = ReadInt(getSettings().rcpath, "bbfoomp.ypos:", 0);
// 		if (getSettings().FooTrans)
// 			SetTransparency(hwndPlugin, (unsigned char)getSettings().transparencyAlpha);
// 		UpdatePosition();
// 	}
// 	else
// 	{
// 		// Since position in slit != position as plugin we need to
// 		// save the current position before docking to the slit...
// 		WriteInt(getSettings().rcpath, "bbfoomp.xpos:", getSettings().xpos);
// 		WriteInt(getSettings().rcpath, "bbfoomp.ypos:", getSettings().ypos);
// 		getSettings().FooDockedToSlit = true;
// 		if (getSettings().FooTrans)
// 			SetTransparency(hwndPlugin, (BYTE)255);
// 		SendMessage(hwndSlit, SLIT_ADD, NULL, (LPARAM)hwndPlugin);
// 	}
// 	WriteBool(getSettings().rcpath, "bbfoomp.dockedtoslit:", getSettings().FooDockedToSlit);
// 
// 	static char msg[MAX_LINE_LENGTH];
// 	static WCHAR wmsg[MAX_LINE_LENGTH];
// 	static char status[9];
// 	if (getSettings().FooDockedToSlit)
// 		sprintf(msg, "bbfoomp -> Docked! (slit mode)", status);
// 	else
// 		sprintf(msg, "bbfoomp -> Undocked! (plugin mode)", status);
// 	bbMB2WC(msg, wmsg, MAX_LINE_LENGTH);
// 	SendMessage(GetBBWnd(), BB_SETTOOLBARLABEL, 0, (LPARAM)wmsg);
}

void TrackMouse ()
{
	TRACKMOUSEEVENT track;
	ZeroMemory(&track,sizeof(track));
	track.cbSize = sizeof(track);
	track.dwFlags = TME_LEAVE;
	track.dwHoverTime = HOVER_DEFAULT;
	track.hwndTrack = hwndPlugin;
	TrackMouseEvent(&track);
}


void CalculateButtonPositions (RECT r)
{
	// NOTE: merge into DispMode controls, perhaps?

	// Generate X, Y coordinates for the shapes
	int sumWidth = 0;
	for (int i = 0; i< e_last_button_item; ++i)
		sumWidth += getSettings().buttons[i].width();
	sumWidth += (e_last_button_item-1) * button_spacing;

	// Position within rect.
	int xpos= (r.right - r.left)/2 - (sumWidth/2);
	int ypos= (r.bottom - r.top)/2 - 3;

	//Adjust position for global rect.
	xpos+= r.left;
	ypos+= r.top;


	for (int i = 0; i < e_last_button_item; ++i)
	{
		getSettings().buttons[i].x = xpos;
		getSettings().buttons[i].y = ypos;
		xpos += getSettings().buttons[i].width() + button_spacing;
	}

	// Generate hit rectangles
	int const rtop = r.top + 1;
	int const rbottom = r.bottom - 1;

	for (int i = 0; i < e_last_button_item; ++i)
	{
		FoompButton &b = getSettings().buttons[i];
		b.hitrect.top = rtop;
		b.hitrect.bottom = rbottom;
		int const padding = (12 - b.width())/2;
		b.hitrect.left = b.x - padding;
		b.hitrect.right = b.x + b.width() + padding;
	}
}

void DispModeControls (RECT r, HDC buf)
{
	for (int i = 0; i < e_last_button_item; ++i)
		getSettings().buttons[i].draw(buf);
}

void ClickMouse (int mouseX, int mouseY)
{
	if (DisplayMode == 2 || DisplayMode == 3)
		for (int i = 0; i < e_last_button_item; ++i)
			if (getSettings().buttons[i].clicked(mouseX,mouseY))
			{
				getSettings().buttons[i].pressed = true;
				InvalidateRect(hwndPlugin, NULL, false);
				return;
			}
}

void Transparency ()
{
	// Transparency is only supported under Windows 2000/XP...
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(osInfo));
	osInfo.dwOSVersionInfoSize = sizeof(osInfo);
	GetVersionEx(&osInfo);

	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osInfo.dwMajorVersion == 5)
		usingWin2kXP = true;
	else
		usingWin2kXP = false;
	if (usingWin2kXP)
	{
		if (getSettings().FooTrans && !getSettings().FooDockedToSlit)
		{	
			SetTransparency(hwndPlugin, (unsigned char)255);
			SetTransparency(hwndPlugin, (unsigned char)getSettings().transparencyAlpha);
		}
	}
}

/// detect foobar2000
struct FindWindowData
{
	FindWindowData(TCHAR const * windowTitle)
		: WindowTitle(windowTitle)
		, ResultHandle(0)
	{}

	std::basic_string<TCHAR> WindowTitle;
	HWND ResultHandle;
};

BOOL CALLBACK FindWindowImpl (HWND hWnd, LPARAM lParam)
{
	FindWindowData * p = reinterpret_cast<FindWindowData*>(lParam);
	if (!p)
		return FALSE;

	int const length = GetWindowTextLength(hWnd) + 1;
	if (length > 0)
	{
		std::vector<TCHAR> buffer(std::size_t(length), 0);
		if (GetWindowText(hWnd, &buffer[0], length))
		{
			if (_tcsstr(&buffer[0], p->WindowTitle.c_str()))
			{
				p->ResultHandle = hWnd;
				return FALSE; // Finish enumerating we found what we need
			}
		}
	}
	return TRUE; // Continue enumerating
}

// Returns the window handle when found if it returns 0 GetLastError() will return more information
HWND FindWindowStart (TCHAR const * windowTitle)
{
	if (!windowTitle)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	FindWindowData data(windowTitle);
	if (!EnumWindows(FindWindowImpl, (LONG_PTR)&data) && data.ResultHandle != 0)
	{
		SetLastError(ERROR_SUCCESS);
		return data.ResultHandle;
	}

	// Return ERROR_FILE_NOT_FOUND in GetLastError
	SetLastError(ERROR_FILE_NOT_FOUND);
	return 0;
}

void Finfo::update ()
{
	// ===== Gets the Handle for FooBar and then uses that to get the windowname.
	// Here is where stuff gets tricky. Foobar by default has a really ugly handle,
	// and plugins such as Foo_UI Columns change the handle name! So we have to figure out
	// if UI Columns is being used and then re-grab the handle. Here goes!
	// *** NOTE: Current support for foobar 9.1 and Columns UI 0.1.3 beta 1v5
	wcscpy(song_title, L"");
	foobar_v9 = false;
	if (FooHandle = FindWindow(TEXT("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}"), NULL)) // Foobar 8.3
	{
		GetWindowTextW(FooHandle, song_title, sizeof(song_title) / sizeof(*song_title));
		if (wcsicmp(song_title, L"uninteresting")==0) // It seems Columns UI 1.2 is loaded for 8.3
		{
			FooHandle = FindWindow(TEXT("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}"), NULL);
			GetWindowTextW(FooHandle, song_title, sizeof(song_title) / sizeof(*song_title));
		}
	}
	else if (FooHandle = FindWindow(TEXT("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1"), NULL)) // Plain foobar 9.1
	{
		foobar_v9 = true;
		GetWindowTextW(FooHandle, song_title, sizeof(song_title) / sizeof(*song_title));
		if (wchar_t *c = wcsstr(song_title,L"	[foobar2000 v0.9.") ) *c = L'\0'; // Cut off trailing text
	}
	else if (FooHandle = FindWindow(TEXT("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}"), NULL)) // Foobar 9.1 with Coloumns UI
	{
		foobar_v9 = true;
		GetWindowTextW(FooHandle, song_title, sizeof(song_title) / sizeof(*song_title));
	}
	else if (FooHandle = FindWindowStart(TEXT("[foobar2000 ")))
	{
		//foobar_v10 = true;
		GetWindowTextW(FooHandle, song_title, sizeof(song_title) / sizeof(*song_title));
	}
	else // If there is no handle (foobar is not on), then display the NoInfoText var.
		wcscpy(song_title, getSettings().NoInfoText.c_str());
}


