/*
 ============================================================================
  bbLua - Lua script engine for BlackBox
  Copyright © 2007 noccy

  This program is free software, released under the GNU General Public
  License (GPL version 2 or later). See:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

 ============================================================================
*/

#define __PLUGIN

//#include "BBApi.h"
#include <blackbox/plugin/bb.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "bblua.h"

bbLua bbLuaInstance;

// ----------------------------------
// plugin info

LPCSTR szVersion        = "bbLua 0.0.1 (BBClean SVN)";
LPCSTR szAppName        = "bbLua";
LPCSTR szInfoVersion    = "0.0.1 (BBClean SVN)";
LPCSTR szInfoAuthor     = "Noccy";
LPCSTR szInfoRelDate    = __DATE__;
LPCSTR szInfoLink       = "http://dev.noccy.com";
LPCSTR szInfoEmail      = "lazerfisk@yahoo.com";


extern "C"
{
	DLL_EXPORT int beginPlugin(HINSTANCE hPluginInstance);
	DLL_EXPORT int beginSlitPlugin(HINSTANCE hPluginInstance, HWND hSlit);
	DLL_EXPORT int beginPluginEx(HINSTANCE hPluginInstance, HWND hSlit);
	DLL_EXPORT void endPlugin(HINSTANCE hPluginInstance);
	DLL_EXPORT LPCSTR pluginInfo(int field);
};

// ----------------------------------
// Global vars

HINSTANCE hInstance;
HWND BBhwnd;
HWND hSlit_present;
bool is_bblean;

// receives the path to "bbSDK.rc"
char rcpath[MAX_PATH];

// ----------------------------------
// Style info

struct style_info
{
	StyleItem Frame;
	int bevelWidth;
	int borderWidth;
	COLORREF borderColor;
} style_info;

// ----------------------------------
// Plugin window properties

struct plugin_properties
{
	// settings
	int xpos, ypos;
	int width, height;

	bool useSlit;
	bool alwaysOnTop;
	bool snapWindow;
	bool pluginToggle;
	bool alphaEnabled;
	bool drawBorder;
	int  alphaValue;

	// our plugin window
	HWND hwnd;

	// current state variables
	bool is_ontop;
	bool is_moving;
	bool is_sizing;
	bool is_hidden;

	// the Slit window, if we are in it.
	HWND hSlit;

	// GDI objects
	HBITMAP bufbmp;
	HFONT hFont;

	// the text
	char window_text[100];

} my;

// ----------------------------------
// some function prototypes

void UpdateStyleInfo();
void GetStyleSettings();
void ReadRCSettings();
void WriteRCSettings();
void ShowMyMenu(bool popup);
void invalidate_window(void);
void set_window_modes(void);

// ConsoleAdapter Console;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ----------------------------------
// helper to handle commands  from the menu

void eval_menu_cmd(int mode, void *pValue, const char *sub_message);
enum eval_menu_cmd_modes
{
	M_BOL = 1,
	M_INT = 2,
	M_STR = 3,
};

//*****************************************************************************
// utilities

// case insensitive string compare, up to lenght of second string
int my_substr_icmp(const char *a, const char *b)
{
	return _memicmp(a, b, strlen(b));
}

// debugging (checkout "DBGVIEW" from "http://www.sysinternals.com/")
void dbg_printf (const char *fmt, ...)
{
	char buffer[4096]; va_list arg;
	va_start(arg, fmt);
	vsprintf (buffer, fmt, arg);
	OutputDebugString(buffer);
}

//===========================================================================

//===========================================================================
// The startup interface

int beginSlitPlugin(HINSTANCE hInstance, HWND hSlit)
{
	return beginPlugin(hInstance);
}

//===========================================================================
// xoblite type slit interface

int beginPluginEx(HINSTANCE hInstance, HWND slit)
{
  return beginPlugin(hInstance);
}

// no-slit interface
int beginPlugin(HINSTANCE hInstance)
{
	// Console.CreateConsole(OUTPUT_CONS);
	// ---------------------------------------------------
	// grab some global information

	BBhwnd          = GetBBWnd();
	//hInstance       = hPluginInstance;
	//hSlit_present   = hSlit;
	is_bblean       = 0 == my_substr_icmp(GetBBVersion(), "bblean");

	// ---------------------------------------------------
	// register the window class

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof wc);

	wc.lpfnWndProc      = WndProc;      // window procedure
	wc.hInstance        = hInstance;    // hInstance of .dll
	wc.lpszClassName    = szAppName;    // window class name
	wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
	wc.style            = CS_DBLCLKS;

	if (!RegisterClass(&wc))
	{
		MessageBox(BBhwnd,
			"Error registering window class", szVersion,
				MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// ---------------------------------------------------
	// Zero out variables, read configuration and style

	ZeroMemory(&my, sizeof my);

	// ReadRCSettings();

	// ---------------------------------------------------
	// create the window

	my.hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,   // window ex-style
		szAppName,          // window class name
		NULL,               // window caption text
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // window style
		my.xpos,            // x position
		my.ypos,            // y position
		my.width,           // window width
		my.height,          // window height
		NULL,               // parent window
		NULL,               // window menu
		hInstance,          // hInstance of .dll
		NULL                // creation data
		);

	bbLuaInstance.init();
	bbLuaInstance.loadFile("bblua.lua");

	return 0;
}

//===========================================================================
// on unload...

void endPlugin(HINSTANCE hPluginInstance)
{
	// Get out of the Slit, in case we are...
	// if (my.hSlit) SendMessage(my.hSlit, SLIT_REMOVE, 0, (LPARAM)my.hwnd);

	bbLuaInstance.close();

	// Destroy the window...
	DestroyWindow(my.hwnd);

	// Unregister window class...
	UnregisterClass(szAppName, hPluginInstance);
}

//===========================================================================
// pluginInfo is used by Blackbox for Windows to fetch information about
// a particular plugin.

LPCSTR pluginInfo(int field)
{
	switch (field)
	{
		case PLUGIN_NAME:           return szAppName;       // Plugin name
		case PLUGIN_VERSION:        return szInfoVersion;   // Plugin version
		case PLUGIN_AUTHOR:         return szInfoAuthor;    // Author
		case PLUGIN_RELEASE:        return szInfoRelDate;   // Release date, preferably in yyyy-mm-dd format
		case PLUGIN_LINK:           return szInfoLink;      // Link to author's website
		case PLUGIN_EMAIL:          return szInfoEmail;     // Author's email
		default:                    return szVersion;       // Fallback: Plugin name + version, e.g. "MyPlugin 1.0"
	}
}


//===========================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int msgs[] = { BB_RECONFIGURE, BB_BROADCAST, 0};

	switch (message)
	{
		case WM_CREATE:
			// Register to reveive these message
			SendMessage(BBhwnd, BB_REGISTERMESSAGE, (WPARAM)hwnd, (LPARAM)msgs);
			// Make the window appear on all workspaces
			// MakeSticky(hwnd);
			break;

		case WM_DESTROY:
			// RemoveSticky(hwnd);
			SendMessage(BBhwnd, BB_UNREGISTERMESSAGE, (WPARAM)hwnd, (LPARAM)msgs);
			break;

		// ----------------------------------------------------------
		// Blackbox sends a "BB_RECONFIGURE" message on style changes etc.

		case BB_RECONFIGURE:
			break;

		// ----------------------------------------------------------
		// prevent the user from closing the plugin with alt-F4

		case WM_CLOSE:
			break;

		// ----------------------------------------------------------
		// let windows handle any other message
		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

//===========================================================================

/*
void ReadRCSettings(void)
{
	int i = 0;
	do
	{
		// First and third, we look for the config file
		// in the same folder as the plugin...
		HINSTANCE hInst = hInstance;
		// second we check the blackbox directory
		if (1 == i) hInst = NULL;

		GetModuleFileName(hInst, rcpath, sizeof(rcpath));
		char *file_name_start = strrchr(rcpath, '\\');
		if (file_name_start) ++file_name_start;
		else file_name_start = strchr(rcpath, 0);
		strcpy(file_name_start, "bbSDK.rc");

	} while (++i < 3 && false == FileExists(rcpath));

	// If a config file was found we read the plugin settings from it...
	// ...if not, the ReadXXX functions give us just the defaults.

	my.xpos     = ReadInt(rcpath, "bbSDK.xpos:", 10);
	my.ypos     = ReadInt(rcpath, "bbSDK.ypos:", 10);
	my.width    = ReadInt(rcpath, "bbSDK.width:", 80);
	my.height   = ReadInt(rcpath, "bbSDK.height:", 40);

	my.alphaEnabled     = ReadBool(rcpath, "bbSDK.alphaEnabled:", false);
	my.alphaValue       = ReadInt(rcpath,  "bbSDK.alphaValue:", 192);
	my.alwaysOnTop      = ReadBool(rcpath, "bbSDK.alwaysOntop:", true);
	my.drawBorder       = ReadBool(rcpath, "bbSDK.drawBorder:", true);
	my.snapWindow       = ReadBool(rcpath, "bbSDK.snapWindow:", true);
	my.pluginToggle     = ReadBool(rcpath, "bbSDK.pluginToggle:", true);
	my.useSlit          = ReadBool(rcpath, "bbSDK.useSlit:", true);

	strcpy(my.window_text, ReadString(rcpath, "bbSDK.windowText:", szAppName));
}
*/

//===========================================================================
