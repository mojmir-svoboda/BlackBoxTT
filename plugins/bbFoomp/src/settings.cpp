#include "settings.h"
#include <blackbox/plugin/bb.h>
#include <tchar.h>
#include <bblibcompat/bblibcompat.h>

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
// 	// NOTE: make a 0.8.3 compatible RC file
// 	const char *default_commands[] =
// 	{
// 		"/prev",
// 		"/play",
// 		"/playpause",
// 		"/stop",
// 		"/next",
// 		"/command:\"Activate or hide\"",
// 		"/command:\"Volume up\"",
// 		"/command:\"Volume down\"",
// 		"/command:\"Open...\""
// 	};
// 	const char *default_altcommands[] =
// 	{
// 		"",
// 		"/rand",
// 		"",
// 		"",
// 		"",
// 		"",
// 		"",
// 		"",
// 		"/command:\"Add files...\""
// 	};
// 
// 
// 	TCHAR pluginDir[MAX_LINE_LENGTH];
// 	int nLen;
// 
// 	// First we extract the plugin directory...
// 	GetModuleFileName(hInstance, pluginDir, sizeof(pluginDir));
// 	nLen = _tcslen(pluginDir)-1;
// 	while (nLen >0 && pluginDir[nLen] != '\\') nLen--;
// 	pluginDir[nLen + 1] = 0;
// 
// 	// ...then we search for the bbfoomp.rc config file...
// 	// (-> $UserAppData$\Blackbox -> plugin directory -> Blackbox directory)
// 	_tcscpy(rcpath, ConfigFileExists("bbfoomp.rc", pluginDir));
// 	if (!_tcslen(rcpath)) strcpy(rcpath, ConfigFileExists("bbfoomprc", pluginDir));
// 	if (!_tcslen(rcpath))
// 	{
// 		// If bbfoomp.rc could not be found we create a new
// 		// config file in the same folder as the plugin...
// 		_tcscpy(rcpath, pluginDir);
// 		_tcscat(rcpath, "bbfoomp.rc");
// 		WriteDefaultRCSettings();
// 	}
// 
// 	//====================
// 
// 	// Read bbfoomp settings from config file...
// #if defined _WIN64
// 	_tcscpy(FooPath, ReadString(rcpath, "bbfoomp.foobar.path:", "C:\\Program Files (x86)\\foobar2000\\foobar2000.exe"));
// #else
// 	_tcscpy(FooPath, ReadString(rcpath, "bbfoomp.foobar.path:", "C:\\Progra~1\\foobar2000\\foobar2000.exe"));
// #endif
// 	const char * def_text = ReadString(rcpath, "bbfoomp.DefaultText:", "Nothing is playing");
// 
// 	bbMB2WC(def_text, NoInfoText, MAX_LINE_LENGTH);
// 	FooWidth = ReadInt(rcpath, "bbfoomp.foowidth:" , 200);
// 	height = ReadInt(rcpath, "bbfoomp.height:", 20);
// 	FooMode = ReadInt(rcpath, "bbfoomp.displaytype:", 2);
// 	InnerStyleIndex = ReadInt(rcpath, "bbfoomp.InnerStyle:", 2);
// 	OuterStyleIndex = ReadInt(rcpath, "bbfoomp.OuterStyle:", 4);
// 	FooOnTop = ReadBool(rcpath, "bbfoomp.OnTop:", false);
// 	transparencyAlpha = ReadInt(rcpath, "bbfoomp.transparencyAlpha:", 220);
// 	BorderWidth = ReadInt(rcpath, "bbfoomp.borderwidth:", 3);
// 	FooTrans = ReadBool(rcpath, "bbfoomp.transparency:", false);
// 	FooAlign = ReadBool(rcpath, "bbfoomp.MegaLeftAlign:", true);
// 	FooShadowsEnabled = ReadBool(rcpath, "bbfoomp.Shadows:", false);
// 	FooScrollSpeed = ReadInt(rcpath, "bbfoomp.ScrollSpeed:", 5);
// 
// 	for (int i = 0; i < e_last_button_item; ++i)
// 	{
// 		FoompButton &b = buttons[i];
// 		char picname[100], cmdname[100], altcmdname[100];
// 		sprintf(picname,"bbfoomp.button%d.image:",i+1);
// 		sprintf(cmdname,"bbfoomp.button%d.command:",i+1);
// 		sprintf(altcmdname,"bbfoomp.button%d.altcommand:",i+1);
// 		b.type = ButtonType(ReadInt(rcpath, picname, i));
// 		_tcscpy(b.cmdarg, ReadString(rcpath, cmdname, default_commands[i]));
// 		_tcscpy(b.altcmdarg, ReadString(rcpath, altcmdname, default_altcommands[i]));
// 	}
// 
// 	xpos = ReadInt(rcpath, "bbfoomp.xpos:", 10);
// 	if (xpos >= GetSystemMetrics(SM_CXSCREEN)) xpos = 0;
// 	ypos = ReadInt(rcpath, "bbfoomp.ypos:", 10);
// 	if (ypos >= GetSystemMetrics(SM_CYSCREEN)) ypos = 0;
// 
// 	if (SlitExists)
// 		FooDockedToSlit = ReadBool(rcpath, "bbfoomp.dockedtoslit:", false);
// 	else
// 		FooDockedToSlit = false;
// 
// 	// Minimum settings checks.
// 	if (height < (15 + BorderWidth) || width < 0 || BorderWidth < 0)
// 	{
// 		MessageBox(0, TEXT("The value you have inputted for either: \nheight, width or border-width is below the minimum.\nThe values will default. Please consult the Readme for the minimums."), "ERROR: Illegal value set.", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
// 		FooWidth = 200;
// 		height = 22;
// 		BorderWidth = 3;
// 	}
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


