/* ------------------------------------------------------------------ *

  bbSDK - Example plugin for Blackbox for Windows.
  Copyright © 2004,2009 grischka

  This program is free software, released under the GNU General Public
  License (GPL version 2). See:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ *

  Description:
  ------------
  This is an example Plugin for Blackbox for Windows. It displays
  a little stylized window with an inscription.

  Left mouse:
   - with the control key held down: moves the plugin
   - with alt key held down: resizes the plugin

  Right mouse click:
	- shows the plugin menu with some standard plugin configuration
	  options. Also the inscription text can be set.

 * ------------------------------------------------------------------ */

#include <blackbox/plugin/bb.h>
#include <bblibcompat/iminmax.h>
#include <bblibcompat/StyleStruct.h>
#include <cstdlib>
#include <cstdio>
#include <bblibcompat/bblibcompat.h>
#include <bblibcompat/StyleStruct.h>
#include <bblib/utils_paths.h>
#include <3rd_party/Assert/Assert.h>
#include <blackbox/BlackBox_compat.h>

#if defined (bbSDK_EXPORTS)
#	define BBSDK_API __declspec(dllexport)
#else
#	define BBSDK_API __declspec(dllimport)
#endif

/* ---------------------------------- */
/* plugin info */

const char szAppName	  [] = "bbSDK";
const char szInfoVersion  [] = "0.2";
const char szInfoAuthor   [] = "grischka";
const char szInfoRelDate  [] = "2016-05-20";
const char szInfoLink	  [] = "http://bb4win.sourceforge.net/bblean";
const char szInfoEmail	  [] = "grischka@users.sourceforge.net";
const char szVersion	  [] = "bbSDK 0.3"; /* fallback for pluginInfo() */
const char szCopyright	  [] = "2004,2009,2016";

const wchar_t szAppNameW[] = L"bbSDK";
const wchar_t szInfoVersionW[] = L"0.2";
const wchar_t szInfoAuthorW[] = L"grischka";
const wchar_t szInfoRelDateW[] = L"2016-05-20";
const wchar_t szInfoLinkW[] = L"http://bb4win.sourceforge.net/bblean";
const wchar_t szInfoEmailW[] = L"grischka@users.sourceforge.net";
const wchar_t szVersionW[] = L"bbSDK 0.3"; /* fallback for pluginInfo() */
const wchar_t szCopyrightW[] = L"2004,2009,2016";

/* ---------------------------------- */
/* The About MessageBox */

void about_box(void)
{
	wchar_t szTemp[1024];
	_snwprintf_s(szTemp, 1024,
		L"%s - A plugin example for Blackbox 4 Windows."
		L"\n© %s %s"
		L"\n%s"
		, szVersionW, szCopyrightW, szInfoEmailW, szInfoLinkW
		);
	MessageBox(NULL, szTemp, L"About", MB_OK | MB_TOPMOST);
}

/* ---------------------------------- */
/* Dependencies on the plugin-name */

/* prefix for our broadcast messages */
#define BROAM_PREFIX L"@bbSDK."
#define BROAM(key) (BROAM_PREFIX key) /* concatenation */

/* configuration file */
#define RC_FILE L"bbSDK.rc"

/* prefix for items in the configuration file */
#define RC_PREFIX L"bbsdk."
#define RC_KEY(key) (RC_PREFIX key ":")

/* prefix for unique menu id's */
#define MENU_ID(key) (L"bbSDK_ID" key)

/* ---------------------------------- */
/* Interface declaration */

extern "C" {
	BBSDK_API int beginPlugin (HINSTANCE hPluginInstance);
	BBSDK_API int beginSlitPlugin (HINSTANCE hPluginInstance, HWND hSlit);
	BBSDK_API int beginPluginEx (HINSTANCE hPluginInstance, HWND hSlit);
	BBSDK_API void endPlugin (HINSTANCE hPluginInstance);
	BBSDK_API LPCSTR pluginInfo (int field);
	BBSDK_API wchar_t const * pluginInfoW (int field);
};

/* ---------------------------------- */
/* Global variables */

HINSTANCE g_hInstance;
HWND g_hSlit;
HWND g_BBhwnd;

/* full path to configuration file */
//bbstring rcpath;
char rcpath[MAX_PATH];

/* ---------------------------------- */
/* Plugin window properties */

struct PluginProperties
{
	/* settings */
	int xpos, ypos;
	int width, height;

	bool useSlit;
	bool alwaysOnTop;
	bool snapWindow;
	bool pluginToggle;
	bool drawBorder;
	bool alphaEnabled;
	int  alphaValue;
	wchar_t windowText[128];

	/* our plugin window */
	HWND hwnd;

	/* current state variables */
	bool is_ontop;
	bool is_moving;
	bool is_sizing;
	bool is_hidden;
	bool is_inslit;

	/* the Style */
	StyleItem Frame;

	/* GDI objects */
	HBITMAP bufbmp;
	HFONT hFont;

	PluginProperties ()
	{
		memset(this, 0, sizeof(*this));
	}

	void ResetToDefaultValues ()
	{
		memset(this, 0, sizeof(*this));
		xpos = 10;
		ypos = 10;
		width = 80;
		height = 40;

		alphaEnabled = false;
		alphaValue = 192;
		alwaysOnTop = true;
		drawBorder = true;
		snapWindow = true;
		pluginToggle = true;
		useSlit = true;

		wcscpy(windowText, szVersionW);
	}
};

/* why are these in a struct and wy is it named 'my'?
   Well, because my.xpos is nicely selfdocumenting. That's all :) */
PluginProperties my;

/* ---------------------------------- */
/* some function prototypes */
void GetStyleSettings ();
void ReadRCSettings ();
void WriteRCSettings ();
void ShowMyMenu (bool popup);
void invalidate_window ();
void set_window_modes ();

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* helper to handle commands from the menu */
struct msg_test
{
	const wchar_t * msg;
	const wchar_t * test;
};
int scan_broam (msg_test * msg_test, wchar_t const * test);
void eval_broam (msg_test * msg_test, int mode, void * pValue);
enum eval_broam_modes
{
	M_BOL = 1,
	M_INT = 2,
	M_STR = 3,
};

/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* The startup interface */

/* slit interface */
int beginPluginEx (HINSTANCE hPluginInstance, HWND hSlit)
{
	WNDCLASS wc;

	/* --------------------------------------------------- */
	/* This plugin can run in one instance only. If BBhwnd is set it means we are already loaded. */
	if (g_BBhwnd)
	{
		MessageBox(g_BBhwnd, L"Do not load me twice!", szVersionW, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1; /* 1 = failure */
	}

	/* --------------------------------------------------- */
	/* grab some global information */
	g_BBhwnd = GetBBWnd();
	g_hInstance = hPluginInstance;
	g_hSlit = hSlit;

	/* --------------------------------------------------- */
	/* register the window class */
	memset(&wc, 0, sizeof wc);
	wc.lpfnWndProc	= WndProc;		/* window procedure */
	wc.hInstance	= g_hInstance;	/* hInstance of .dll */
	wc.lpszClassName = szAppNameW;	  /* window class name */
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.style		= CS_DBLCLKS;

	if (!RegisterClass(&wc))
	{
		MessageBox(g_BBhwnd, L"Error registering window class", szVersionW, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1; /* 1 = failure */
	}

	/* --------------------------------------------------- */
	/* Zero out variables, read configuration and style */
	my.ResetToDefaultValues();
	ReadRCSettings();
	GetStyleSettings();

	/* --------------------------------------------------- */
	/* create the window */
	my.hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,	/* window ex-style */
		szAppNameW,			 /* window class name */
		NULL,				/* window caption text */
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, /* window style */
		0,					/* x position */
		0,					/* y position */
		0,					/* window width */
		0,					/* window height */
		NULL,				/* parent window */
		NULL,				/* window menu */
		g_hInstance,		/* hInstance of .dll */
		NULL				/* creation data */
		);

	/* set window location and properties */
	set_window_modes();

	/* show window (without stealing focus) */
	ShowWindow(my.hwnd, SW_SHOWNA);
	return 0; /* 0 = success */
}

/* no-slit interface */
int beginPlugin (HINSTANCE hPluginInstance)
{
	return beginPluginEx(hPluginInstance, NULL);
}

/* ------------------------------------------------------------------ */
/* on unload... */
void endPlugin (HINSTANCE hPluginInstance)
{
	/* Get out of the Slit, in case we are... */
	if (my.is_inslit)
		SendMessage(g_hSlit, SLIT_REMOVE, 0, (LPARAM)my.hwnd);

	/* Destroy the window... */
	::DestroyWindow(my.hwnd);

	/* clean up HBITMAP object */
	if (my.bufbmp)
		DeleteObject(my.bufbmp);

	/* clean up HFONT object */
	if (my.hFont)
		DeleteObject(my.hFont);

	/* Unregister window class... */
	UnregisterClass(szAppNameW, hPluginInstance);
}

/* ------------------------------------------------------------------ */
/* pluginInfo is used by Blackbox for Windows to fetch information about a particular plugin. */
LPCSTR pluginInfo (int index)
{
	switch (index)
	{
		case PLUGIN_NAME:		return szAppName;		/* Plugin name */
		case PLUGIN_VERSION:	return szInfoVersion;	/* Plugin version */
		case PLUGIN_AUTHOR:		return szInfoAuthor;	/* Author */
		case PLUGIN_RELEASE:	return szInfoRelDate;	/* Release date, preferably in yyyy-mm-dd format */
		case PLUGIN_LINK:		return szInfoLink;		/* Link to author's website */
		case PLUGIN_EMAIL:		return szInfoEmail;		/* Author's email */
		default:				return szVersion;		/* Fallback: Plugin name + version, e.g. "MyPlugin 1.0" */
	}
}
/* ------------------------------------------------------------------ */
/* pluginInfoW is used by Blackbox for Windows to fetch information about a particular plugin in wide chars */
wchar_t const * pluginInfoW (int index)
{
	switch (index)
	{
		case PLUGIN_NAME:		return szAppNameW;		/* Plugin name */
		case PLUGIN_VERSION:	return szInfoVersionW;	/* Plugin version */
		case PLUGIN_AUTHOR:		return szInfoAuthorW;	/* Author */
		case PLUGIN_RELEASE:	return szInfoRelDateW;	/* Release date, preferably in yyyy-mm-dd format */
		case PLUGIN_LINK:		return szInfoLinkW;		/* Link to author's website */
		case PLUGIN_EMAIL:		return szInfoEmailW;		/* Author's email */
		default:				return szVersionW;		/* Fallback: Plugin name + version, e.g. "MyPlugin 1.0" */
	}
}


/* ------------------------------------------------------------------ */
/* utilities */

/* debugging utility - 'DBGVIEW' from http://www.sysinternals.com/ may be used to catch the output (or similar tools). */
void dbg_printf (const wchar_t *fmt, ...)
{
	wchar_t buffer[4000];
	va_list arg;

	va_start(arg, fmt);
	vswprintf_s(buffer, _TRUNCATE, fmt, arg);
	OutputDebugString(buffer);
}

/* edit a file with the blackbox editor */
void edit_rc(const char *path)
{
//	   if (under_bblean) {
//		   SendMessage(BBhwnd, BB_EDITFILE, (WPARAM)-1, (LPARAM)path);
//	   } else {
//		   char editor[MAX_PATH];
//		   GetBlackboxEditor(editor);
//		   BBExecute(NULL, NULL, editor, path, NULL, SW_SHOWNORMAL, false);
//	   }
}

/* this invalidates the window, and resets the bitmap at the same time. */
void invalidate_window(void)
{
	if (my.bufbmp)
	{
		/* delete the double buffer bitmap (if we have one), so it
		   will be drawn again next time with WM_PAINT */
		DeleteObject(my.bufbmp);
		my.bufbmp = NULL;
	}
	/* notify the OS that the window needs painting */
	InvalidateRect(my.hwnd, NULL, FALSE);
}

/* ------------------------------------------------------------------ */
/* paint the window into the buffer HDC */
void paint_window (HDC hdc_buffer, RECT * p_rect)
{
	/* and draw the frame */
	MakeStyleGradient(hdc_buffer, p_rect, &my.Frame, my.drawBorder);

	if (my.windowText[0])
	{
		HGDIOBJ otherfont;
		int margin;
		RECT text_rect;

		/* Set the font, storing the default.. */
		otherfont = SelectObject(hdc_buffer, my.hFont);

		/* adjust the rectangle */
		margin = my.Frame.marginWidth + my.Frame.bevelposition;
		if (my.drawBorder)
			margin += my.Frame.borderWidth;

		text_rect.left	= p_rect->left + margin;
		text_rect.top	= p_rect->top + margin;
		text_rect.right = p_rect->right - margin;
		text_rect.bottom = p_rect->bottom - margin;

		/* draw the text */
	  SetTextColor(hdc_buffer, my.Frame.TextColor);
		SetBkMode(hdc_buffer, TRANSPARENT);
		DrawTextW(hdc_buffer, my.windowText, -1, &text_rect, my.Frame.Justify|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

		/* Put back the previous default font. */
		SelectObject(hdc_buffer, otherfont);
	}
}

/* ------------------------------------------------------------------ */

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int msgs[] = { BB_RECONFIGURE, BB_BROADCAST, 0};

	switch (message)
	{
		case WM_CREATE:
			/* Register to reveive these message */
			SendMessage(g_BBhwnd, BB_REGISTERMESSAGE, (WPARAM)hwnd, (LPARAM)msgs);
			/* Make the window appear on all workspaces */
			MakeSticky(hwnd);
			break;

		case WM_DESTROY:
			/* as above, in reverse */
			RemoveSticky(hwnd);
			SendMessage(g_BBhwnd, BB_UNREGISTERMESSAGE, (WPARAM)hwnd, (LPARAM)msgs);
			break;

		/* ---------------------------------------------------------- */
		/* Blackbox sends a "BB_RECONFIGURE" message on style changes etc. */

		case BB_RECONFIGURE:
			ReadRCSettings();
			GetStyleSettings();
			set_window_modes();
			break;

		/* ---------------------------------------------------------- */
		/* Painting directly on screen. Good enough for static plugins. */
#if 1
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT r;

			/* get screen DC */
			hdc = BeginPaint(hwnd, &ps);

			/* Setup the rectangle */
			r.left = r.top = 0;
			r.right = my.width;
			r.bottom =	my.height;

			/* and paint everything on it*/
			paint_window(hdc, &r);

			/* Done */
			EndPaint(hwnd, &ps);
			break;
		}

		/* ---------------------------------------------------------- */
		/* Painting with a cached double-buffer. If your plugin updates
		   frequently, this avoids flicker */
#else
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc, hdc_buffer;
			HGDIOBJ otherbmp;
			RECT r;

			/* get screen DC */
			hdc = BeginPaint(hwnd, &ps);

			/* create a DC for the buffer */
			hdc_buffer = CreateCompatibleDC(hdc);

			if (NULL == my.bufbmp) /* No bitmap yet? */
			{
				/* Make a bitmap ... */
				my.bufbmp = CreateCompatibleBitmap(hdc, my.width, my.height);

				/* ... and select it into the DC, saving the previous default. */
				otherbmp = SelectObject(hdc_buffer, my.bufbmp);

				/* Setup the rectangle */
				r.left = r.top = 0;
				r.right = my.width;
				r.bottom =	my.height;

				/* and paint everything on it*/
				paint_window(hdc_buffer, &r);
			}
			else
			{
				/* Otherwise it has been painted already,
				   so just select it into the DC */
				otherbmp = SelectObject(hdc_buffer, my.bufbmp);
			}

			/* Copy the buffer on the screen, within the invalid rectangle: */
			BitBltRect(hdc, hdc_buffer, &ps.rcPaint);

			/* Put back the previous default bitmap */
			SelectObject(hdc_buffer, otherbmp);
			/* clean up */
			DeleteDC(hdc_buffer);

			/* Done. */
			EndPaint(hwnd, &ps);
			break;
		}
#endif
		/* ---------------------------------------------------------- */
		/* Manually moving/sizing has been started */

		case WM_ENTERSIZEMOVE:
			my.is_moving = true;
			break;

		case WM_EXITSIZEMOVE:
			if (my.is_moving)
			{
				if (my.is_inslit)
				{
					/* moving in the slit is not really supported but who
					   knows ... */
					SendMessage(g_hSlit, SLIT_UPDATE, 0, (LPARAM)hwnd);
				}
				else
				{
					/* if not in slit, record new position */
//					   WriteInt(rcpath, RC_KEY("xpos"), my.xpos);
//					   WriteInt(rcpath, RC_KEY("ypos"), my.ypos);
				}

				if (my.is_sizing)
				{
					/* record new size */
//					   WriteInt(rcpath, RC_KEY("width"), my.width);
//					   WriteInt(rcpath, RC_KEY("height"), my.height);
				}
			}
			my.is_moving = my.is_sizing = false;
			set_window_modes();
			break;

		/* --------------------------------------------------- */
		/* snap to edges on moving */

		case WM_WINDOWPOSCHANGING:
			if (my.is_moving)
			{
				WINDOWPOS* wp = (WINDOWPOS*)lParam;
//				   if (my.snapWindow && false == my.is_sizing)
//					   SnapWindowToEdge(wp, 10, SNAP_FULLSCREEN);

				/* set a minimum size */
				if (wp->cx < 40)
					wp->cx = 40;

				if (wp->cy < 20)
					wp->cy = 20;
			}
			break;

		/* --------------------------------------------------- */
		/* record new position or size */

		case WM_WINDOWPOSCHANGED:
			if (my.is_moving)
			{
				WINDOWPOS* wp = (WINDOWPOS*)lParam;
				if (my.is_sizing)
				{
					/* record sizes */
					my.width = wp->cx;
					my.height = wp->cy;

					/* redraw window */
					invalidate_window();
				}

				if (false == my.is_inslit)
				{
					/* record position, if not in slit */
					my.xpos = wp->x;
					my.ypos = wp->y;
				}
			}
			break;

		/* ---------------------------------------------------------- */
		/* start moving or sizing accordingly to keys held down */

		case WM_LBUTTONDOWN:
			UpdateWindow(hwnd);
			if (GetAsyncKeyState(VK_MENU) & 0x8000)
			{
				/* start sizing, when alt-key is held down */
				PostMessage(hwnd, WM_SYSCOMMAND, 0xf008, 0);
				my.is_sizing = true;
			}
			else
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				/* start moving, when control-key is held down */
				PostMessage(hwnd, WM_SYSCOMMAND, 0xf012, 0);
			}
			break;

		/* ---------------------------------------------------------- */
		/* normal mouse clicks */

		case WM_LBUTTONUP:
			/* code goes here ... */
			break;

		case WM_RBUTTONUP:
			/* Show the user menu on right-click (might test for control-key
			   held down if wanted */
			/* if (wParam & MK_CONTROL) */
			ShowMyMenu(true);
			break;

		case WM_LBUTTONDBLCLK:
			/* Do something here ... */
			about_box();
			break;

		/* ---------------------------------------------------------- */
		/* Blackbox sends Broams to all windows... */

		case BB_BROADCAST:
		{
			const wchar_t * msg = (LPWSTR)lParam;
			struct msg_test msg_test;

			/* check general broams */
			if (!_wcsicmp(msg, L"@BBShowPlugins"))
			{
				if (my.is_hidden)
				{
					my.is_hidden = false;
					ShowWindow(hwnd, SW_SHOWNA);
				}
				break;
			}

			if (!_wcsicmp(msg, L"@BBHidePlugins"))
			{
				if (my.pluginToggle && false == my.is_inslit)
				{
					my.is_hidden = true;
					ShowWindow(hwnd, SW_HIDE);
				}
				break;
			}

			/* if the broam is not for us, return now */
			if (0 != _wcsnicmp(msg, BROAM_PREFIX, wcslen(BROAM_PREFIX)))
				break;

			msg_test.msg = msg + wcslen(BROAM_PREFIX);

			if (scan_broam(&msg_test, L"useSlit"))
			{
				eval_broam(&msg_test, M_BOL, &my.useSlit);
				break;
			}

			if (scan_broam(&msg_test, L"alwaysOnTop"))
			{
				eval_broam(&msg_test, M_BOL, &my.alwaysOnTop);
				break;
			}

			if (scan_broam(&msg_test, L"drawBorder"))
			{
				eval_broam(&msg_test, M_BOL, &my.drawBorder);
				break;
			}

			if (scan_broam(&msg_test, L"snapWindow"))
			{
				eval_broam(&msg_test, M_BOL, &my.snapWindow);
				break;
			}

			if (scan_broam(&msg_test, L"pluginToggle"))
			{
				eval_broam(&msg_test, M_BOL, &my.pluginToggle);
				break;
			}

			if (scan_broam(&msg_test, L"alphaEnabled"))
			{
				eval_broam(&msg_test, M_BOL, &my.alphaEnabled);
				break;
			}

			if (scan_broam(&msg_test, L"alphaValue"))
			{
				eval_broam(&msg_test, M_INT, &my.alphaValue);
				break;
			}

			if (scan_broam(&msg_test, L"windowText"))
			{
				eval_broam(&msg_test, M_STR, &my.windowText);
				break;
			}

			if (scan_broam(&msg_test, L"editRC"))
			{
				edit_rc(rcpath);
				break;
			}

			if (scan_broam(&msg_test, L"About"))
			{
				about_box();
				break;
			}

			break;
		}

		/* ---------------------------------------------------------- */
		/* prevent the user from closing the plugin with alt-F4 */

		case WM_CLOSE:
			break;

		/* ---------------------------------------------------------- */
		/* let windows handle any other message */
		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* Update position and size, as well as onTop, transparency and
   inSlit states. */

void set_window_modes(void)
{
	HWND hwnd = my.hwnd;

	/* do we want to use the slit and is there a slit at all?  */
	if (my.useSlit && g_hSlit)
	{
		/* if in slit, dont move... */
		SetWindowPos(hwnd, NULL,
			0, 0, my.width, my.height,
			SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOZORDER|SWP_NOMOVE
			);

		if (my.is_inslit)
		{
			/* we are already in the slit, so send update */
			SendMessage(g_hSlit, SLIT_UPDATE, 0, (LPARAM)hwnd);
		}
		else
		{
			/* transparency must be off in slit */
	   SetTransparency(hwnd, 255);
			/* enter slit now */
			my.is_inslit = true;
			SendMessage(g_hSlit, SLIT_ADD, 0, (LPARAM)hwnd);
		}
	}
	else
	{
		HWND hwnd_after = NULL;
		UINT flags = SWP_NOACTIVATE|SWP_NOSENDCHANGING|SWP_NOZORDER;
		RECT screen_rect;

		if (my.is_inslit)
		{
			/* leave it */
			SendMessage(g_hSlit, SLIT_REMOVE, 0, (LPARAM)hwnd);
			my.is_inslit = false;
		}

		if (my.is_ontop != my.alwaysOnTop)
		{
			my.is_ontop = my.alwaysOnTop;
			hwnd_after = my.is_ontop ? HWND_TOPMOST : HWND_NOTOPMOST;
			flags = SWP_NOACTIVATE|SWP_NOSENDCHANGING;
		}

		// make shure the plugin is on the screen:
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		my.xpos = iminmax(my.xpos, screen_rect.left, screen_rect.right - my.width);
		my.ypos = iminmax(my.ypos, screen_rect.top, screen_rect.bottom - my.height);

		SetWindowPos(hwnd, hwnd_after, my.xpos, my.ypos, my.width, my.height, flags);
	  SetTransparency(hwnd, (BYTE)(my.alphaEnabled ? my.alphaValue : 255));
	}

	/* window needs drawing */
	invalidate_window();
}

/* ------------------------------------------------------------------ */
/* Locate the configuration file */

/* this shows how to 'delay-load' an API that is in one branch but maybe
   not in another */

bool FindRCFile(char* pszOut, const char* rcfile, HINSTANCE plugin_instance)
{
//	   bool (*pFindRCFile)(LPSTR rcpath, LPCSTR rcfile, HINSTANCE plugin_instance);
// 
//	   /* try to grab the function from blackbox.exe */
//	   *(FARPROC*)&pFindRCFile = GetProcAddress(GetModuleHandle(NULL), "FindRCFile");
//	   if (pFindRCFile) {
//		   /* use if present */
//		   return pFindRCFile(rcpath, rcfile, plugin_instance);
// 
//	   } else {
//		  /* otherwise do something similar */
//		  int len = GetModuleFileName(plugin_instance, pszOut, MAX_PATH);
//		  while (len && pszOut[len-1] != '\\')
//			  --len;
//		  strcpy(pszOut + len, rcfile);
//		  return FileExists(pszOut);
//	   }
	return false;
}

/* ------------------------------------------------------------------ */
/* Read the configuration file */

void ReadRCSettings(void)
{
//	   /* Locate configuration file */
//	   FindRCFile(rcpath, RC_FILE, g_hInstance);
// 
//	   /* Read our settings. (If the config file does not exist,
//		  the Read... functions give us just the defaults.) */
// 
//	   my.xpos	 = ReadInt(rcpath, RC_KEY("xpos"), 10);
//	   my.ypos	 = ReadInt(rcpath, RC_KEY("ypos"), 10);
//	   my.width  = ReadInt(rcpath, RC_KEY("width"), 80);
//	   my.height = ReadInt(rcpath, RC_KEY("height"), 40);
// 
//	   my.alphaEnabled	 = ReadBool(rcpath, RC_KEY("alphaEnabled"), false);
//	   my.alphaValue	 = ReadInt(rcpath,	RC_KEY("alphaValue"), 192);
//	   my.alwaysOnTop	 = ReadBool(rcpath, RC_KEY("alwaysOntop"), true);
//	   my.drawBorder	 = ReadBool(rcpath, RC_KEY("drawBorder"), true);
//	   my.snapWindow	 = ReadBool(rcpath, RC_KEY("snapWindow"), true);
//	   my.pluginToggle	 = ReadBool(rcpath, RC_KEY("pluginToggle"), true);
//	   my.useSlit		 = ReadBool(rcpath, RC_KEY("useSlit"), true);
// 
//	   strcpy(my.windowText, ReadString(rcpath, RC_KEY("windowText"), szVersion));
}

/* ------------------------------------------------------------------ */
/* Get some blackbox style */
void GetStyleSettings(void)
{
	   my.Frame = *(StyleItem *)GetSettingPtr(SN_TOOLBAR);
	   if (my.hFont)
		   DeleteObject(my.hFont);
	   my.hFont = CreateStyleFont(&my.Frame);
}

/* ------------------------------------------------------------------ */
/* Show or update configuration menu */
void ShowMyMenu(bool popup)
{
	std::shared_ptr<bb::MenuConfig> pMenu, pSub;

	/* Create the main menu, with a title and an unique IDString */
	pMenu = MakeNamedMenu(szAppNameW, MENU_ID(L"Main"), popup);

	/* Create a submenu, also with title and unique IDString */
	pSub = MakeNamedMenu(L"Configuration", MENU_ID(L"Config"), popup);

	/* Insert first Item */
	MakeMenuItemBool(pSub, L"Draw Border", BROAM(L"drawBorder"), my.drawBorder);

	if (g_hSlit)
		MakeMenuItemBool(pSub, L"Use Slit", BROAM(L"useSlit"), my.useSlit);

	if (false == my.is_inslit)
	{
		/* these are only available if outside the slit */
		MakeMenuItemBool(pSub, L"Always On Top", BROAM(L"alwaysOnTop"), my.alwaysOnTop);
		MakeMenuItemBool(pSub, L"Snap To Edges", BROAM(L"snapWindow"), my.snapWindow);
		MakeMenuItemBool(pSub, L"Toggle With Plugins", BROAM(L"pluginToggle"), my.pluginToggle);
		MakeMenuItemBool(pSub, L"Transparency", BROAM(L"alphaEnabled"), my.alphaEnabled);
		MakeMenuItemInt(pSub, L"Alpha Value", BROAM(L"alphaValue"), my.alphaValue, 0, 255);
	}

	/* Insert the submenu into the main menu */
	MakeSubmenu(pMenu, pSub, L"Configuration");

	/* The configurable text string */
	MakeMenuItemString(pMenu, L"Display Text", BROAM(L"windowText"), my.windowText);

	/* ---------------------------------- */
	/* add an empty line */
	MakeMenuNOP(pMenu, NULL);

	/* add an entry to let the user edit the setting file */
	MakeMenuItem(pMenu, L"Edit Settings", BROAM(L"editRC"));

	/* and an about box */
	MakeMenuItem(pMenu, L"About", BROAM(L"About"));

	/* ---------------------------------- */
	/* Finally, show the menu... */
	if (popup)
		ShowMenu(pMenu);
	else
		UpdateMenu(pMenu);
}

/* ------------------------------------------------------------------ */
/* helper to handle commands from the menu */

int scan_broam(msg_test * msg_test, wchar_t const * test)
{
	int const len = wcslen(test);
	wchar_t const * msg = msg_test->msg;

	if (_wcsnicmp(msg, test, len) != 0)
		return 0;

	msg += len;
	if (*msg != 0 && *msg != L' ')
		return 0;

	/* store for function below */
	msg_test->msg = msg;
	msg_test->test = test;
	return 1;
}

void eval_broam(struct msg_test *msg_test, int mode, void *pValue)
{
	wchar_t rc_key[80];

	/* Build the full rc_key. i.e. "@bbSDK.xxx:" */
	wsprintf(rc_key, L"%s%s:", RC_PREFIX, msg_test->test);

	wchar_t const * msg = msg_test->msg;
	/* skip possible whitespace after broam */
	while (*msg == L' ')
		++msg;

	switch (mode)
	{
		/* --- set boolean variable ---------------- */
		case M_BOL:
			if (0 == _wcsicmp(msg, L"true"))
				*(bool*)pValue = true;
			else
			if (0 == _wcsicmp(msg, L"false"))
				*(bool*)pValue = false;
			else
				/* just toggle */
				*(bool*)pValue = false == *(bool*)pValue;

			/* write the new setting to the rc - file */
//			WriteBool(rcpath, rc_key, *(bool*)pValue);
			break;

		/* --- set integer variable ------------------- */
		case M_INT:
			*(int*)pValue = _wtoi(msg);

			/* write the new setting to the rc - file */
//			WriteInt(rcpath, rc_key, *(int*)pValue);
			break;

		/* --- set string variable ------------------- */
		case M_STR:
//			wcscpy((wchar_t*)pValue, msg);

			/* write the new setting to the rc - file */
//			WriteString(rcpath, rc_key, (char*)pValue);
			break;
	}

	/* Apply new settings */
	set_window_modes();

	/* Update the menu checkmarks */
	ShowMyMenu(false);
}

/* ------------------------------------------------------------------ */
