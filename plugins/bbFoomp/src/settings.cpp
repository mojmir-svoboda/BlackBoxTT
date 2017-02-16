#include "settings.h"
#include <blackbox/plugin/bb.h>
#include <tchar.h>
#include <bblibcompat/bblibcompat.h>
#include <bblibcompat/bbPlugin.h>

Settings g_settings;
Settings & getSettings () { return g_settings; } // "singleton"

extern HINSTANCE hInstance;
extern bool SlitExists;
extern HWND hwndPlugin, hwndSlit;

//===========================================================================

void Settings::WriteDefaultRCSettings ()
{
	static char szTemp[MAX_LINE_LENGTH];
	DWORD retLength = 0;

	// Create a new bbfoomp.rc configuration file with default settings...
	HANDLE file = CreateFile(rcpath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		strcpy(szTemp,
#if defined _WIN64
	  "bbfoomp.foobar.path: C:\\Program Files (x86)\\foobar2000\\foobar2000.exe\r\n" // Foo Directory [FooPath]
#else
	  "bbfoomp.foobar.path: C:\\Progra~1\\foobar2000\\foobar2000.exe\r\n" // Foo Directory [FooPath]
#endif
			"bbfoomp.displaytype: 2\r\n"										// FooMode (Mouse Over Mode) [FooMode]
			"bbfoomp.foowidth: 200\r\n"											// FooWidth [self-explanatory]
			"bbfoomp.height: 22\r\n"											// Height [self-explanatory]
			"bbfoomp.borderwidth: 3\r\n"										// Border width [self-explanatory]
			"bbfoomp.xpos: 0\r\n"												// Xpos [x-coordinate]
			"bbfoomp.ypos: 0\r\n"												// Ypos [y-coordinate]
			"bbfoomp.dockedtoslit: false\r\n"									// FooDockedToSlit [self-explanatory]
			"bbfoomp.OnTop: false\r\n"											// FooOnTop [Always On Top]
			"bbfoomp.transparency: false\r\n"									// FooTrans [Transparency]
			"bbfoomp.transparencyAlpha: 220\r\n"								// transparencyAlpha [Amount of Transparency]
			"bbfoomp.RectangleStyle: 2\r\n"										// Inset Rectangle Style [Style in which its drawn]
			"bbfoomp.MegaLeftAlign: true\r\n"									// MegaMode alignment [FooAlign]
			"bbfoomp.DefaultText: Nothing is playing\r\n");						// Text to show when nothing is playing [NoInfoText]

		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
	}
	CloseHandle(file);
}

void Settings::ReadRCSettings ()
{
	// NOTE: make a 0.8.3 compatible RC file
	const wchar_t *default_commands[] =
	{
		L"/prev",
		L"/play",
		L"/playpause",
		L"/stop",
		L"/next",
		L"/command:\"Activate or hide\"",
		L"/command:\"Volume up\"",
		L"/command:\"Volume down\"",
		L"/command:\"Open...\""
	};
	const wchar_t *default_altcommands[] =
	{
		L"",
		L"/rand",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"/command:\"Add files...\""
	};


	TCHAR pluginDir[MAX_LINE_LENGTH];
	int nLen;

	// First we extract the plugin directory...
	GetModuleFileName(hInstance, pluginDir, sizeof(pluginDir));
	nLen = _tcslen(pluginDir)-1;
	while (nLen >0 && pluginDir[nLen] != '\\') nLen--;
	pluginDir[nLen + 1] = 0;

	// ...then we search for the bbfoomp.rc config file...
	// (-> $UserAppData$\Blackbox -> plugin directory -> Blackbox directory)
	if (rcpath.empty())
		BBP_get_rcpath(rcpath, hInstance, L"bbfoomp.rc");

	if (rcpath.empty())
		BBP_get_rcpath(rcpath, hInstance, L"bbfoomprc");
	
// 	if (rcpath.empty())
// 	{
// 		// If bbfoomp.rc could not be found we create a new
// 		// config file in the same folder as the plugin...
// 		_tcscpy(rcpath, pluginDir);
// 		_tcscat(rcpath, "bbfoomp.rc");
// 		WriteDefaultRCSettings();
// 	}

	//====================

	// Read bbfoomp settings from config file...
#if defined _WIN64
	const wchar_t * path = ReadString(rcpath.c_str(), L"bbfoomp.foobar.path", L"C:\\Program Files (x86)\\foobar2000\\foobar2000.exe");
	FooPath = bbstring(path);
#else
	_tcscpy(FooPath, ReadString(rcpath, "bbfoomp.foobar.path", "C:\\Progra~1\\foobar2000\\foobar2000.exe"));
#endif
	NoInfoText = bbstring(ReadString(rcpath.c_str(), L"bbfoomp.DefaultText", L"Nothing is playing"));

	FooWidth = ReadInt(rcpath.c_str(), L"bbfoomp.foowidth" , 200);
	height = ReadInt(rcpath.c_str(), L"bbfoomp.height", 20);
	FooMode = ReadInt(rcpath.c_str(), L"bbfoomp.displaytype", 2);
	InnerStyleIndex = ReadInt(rcpath.c_str(), L"bbfoomp.InnerStyle", 2);
	OuterStyleIndex = ReadInt(rcpath.c_str(), L"bbfoomp.OuterStyle", 4);
	FooOnTop = ReadBool(rcpath.c_str(), L"bbfoomp.OnTop", false);
	transparencyAlpha = ReadInt(rcpath.c_str(), L"bbfoomp.transparencyAlpha", 220);
	BorderWidth = ReadInt(rcpath.c_str(), L"bbfoomp.borderwidth", 3);
	FooTrans = ReadBool(rcpath.c_str(), L"bbfoomp.transparency", false);
	FooAlign = ReadBool(rcpath.c_str(), L"bbfoomp.MegaLeftAlign", true);
	FooShadowsEnabled = ReadBool(rcpath.c_str(), L"bbfoomp.Shadows", false);
	FooScrollSpeed = ReadInt(rcpath.c_str(), L"bbfoomp.ScrollSpeed", 5);

	for (int i = 0; i < e_last_button_item; ++i)
	{
		FoompButton & b = buttons[i];
		wchar_t picname[100], cmdname[100], altcmdname[100];
		wsprintf(picname, L"bbfoomp.button%d.image", i+1);
		wsprintf(cmdname, L"bbfoomp.button%d.command", i+1);
		wsprintf(altcmdname, L"bbfoomp.button%d.altcommand", i+1);
		b.type = static_cast<ButtonType>(ReadInt(rcpath.c_str(), picname, i));
		b.cmdarg = bbstring(ReadString(rcpath.c_str(), cmdname, default_commands[i]));
		b.altcmdarg = bbstring(ReadString(rcpath.c_str(), altcmdname, default_altcommands[i]));
	}

	xpos = ReadInt(rcpath.c_str(), L"bbfoomp.xpos", 10);
	if (xpos >= GetSystemMetrics(SM_CXSCREEN)) xpos = 0;
	ypos = ReadInt(rcpath.c_str(), L"bbfoomp.ypos", 10);
	if (ypos >= GetSystemMetrics(SM_CYSCREEN)) ypos = 0;

	if (SlitExists)
		FooDockedToSlit = ReadBool(rcpath.c_str(), L"bbfoomp.dockedtoslit", false);
	else
		FooDockedToSlit = false;

	// Minimum settings checks.
	if (height < (15 + BorderWidth) || width < 0 || BorderWidth < 0)
	{
		MessageBox(0, TEXT("The value you have inputted for either: \nheight, width or border-width is below the minimum.\nThe values will default. Please consult the Readme for the minimums."), L"ERROR: Illegal value set.", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
		FooWidth = 200;
		height = 22;
		BorderWidth = 3;
	}
}

void Settings::WriteRCSettings ()
{
// 	// Write current position to the config file if *not* docked to the slit...
// 	if (!FooDockedToSlit)
// 	{
// 		WriteInt(rcpath, "bbfoomp.xpos:", xpos);
// 		WriteInt(rcpath, "bbfoomp.ypos:", ypos);
// 
// 		WriteInt(rcpath, "bbfoomp.borderwidth:", BorderWidth);
// 	}
// 	else SendMessage(hwndSlit, SLIT_REMOVE, NULL, (LPARAM)hwndPlugin);
// 
// 	// Writing of following vars:
// 	// FooMode (Mouse Over Mode), FooWidth (Width) and Height (Height).
// 	WriteInt(rcpath, "bbfoomp.displaytype:", FooMode);
// 	WriteInt(rcpath, "bbfoomp.FooWidth:", FooWidth);
// 	WriteInt(rcpath, "bbfoomp.height:", height);
// 
// 	// Write custom button commands.
// 	for (int i = 0; i < e_last_button_item; ++i)
// 	{
// 		FoompButton &b = buttons[i];
// 		char picname[100], cmdname[100], altcmdname[100];
// 		sprintf(picname,"bbfoomp.button%d.image:",i+1);
// 		sprintf(cmdname,"bbfoomp.button%d.command:",i+1);
// 		sprintf(altcmdname,"bbfoomp.button%d.altcommand:",i+1);
// 		WriteInt(rcpath, picname, b.type);
// 		WriteString(rcpath, cmdname, b.cmdarg);
// 		WriteString(rcpath, altcmdname, b.altcmdarg);
// 	}
}


//===========================================================================


