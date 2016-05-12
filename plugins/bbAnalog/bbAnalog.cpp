	/*
	----------------------------------------------------------

	bbAnalog is a plugin for Blackbox for Windows

	Copyright 2004-2009 grischka - grischka@users.sourceforge,net
	Copyright 2003 Mortar - Brad Bartolucci

	bbAnalog is free software, released under the GNU General
	Public License (GPL version 2). For details see:

			http://www.fsf.org/licenses/gpl.html


	   THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT
	  WILL BE USEFUL, BUT WITHOUT ANY WARRANTY; WITHOUT
	   EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR
			FITNESS FOR A PARTICULAR PURPOSE.

	----------------------------------------------------------

	Mouse Input:
	------------

	A few things can be done with simple clicks on the
	bbAnalog window:

	Left Click				Show date for 3 seconds.
	Double Left Click		Displays the Date/Time control panel.
	Right Click				Opens configuration menu
	Ctrl + Left Button		Move bbAnalog (if outside the slit)


	Configuration:
	--------------

	All configuration can be done from the clock's menu.

	One thing to note: Unlike other plugins bbAnalog 1.0AA
	allows you to load as many instances as you like.

	----------------------------------------------------------
*/

#include <bblibcompat/bblibcompat.h>
#include <bblibcompat/bbPlugin.h>
#include <bblibcompat/StyleItem.h>
#include <bblibcompat/StyleStruct.h>
#include <bblibcompat/iminmax.h>
#include <blackbox/cmd/Commands.h>
#include <blackbox/bind/bind.h>
#include <blackbox/plugin/plugin.h>

#if defined (bbAnalog_EXPORTS)
#	define BBANALOG_API __declspec(dllexport)
#else
#	define BBANALOG_API __declspec(dllimport)
#endif

extern "C"
{
	BBANALOG_API int beginPlugin (HINSTANCE hMainInstance);
	BBANALOG_API int beginPluginEx (HINSTANCE hMainInstance, HWND hBBSlit);
	BBANALOG_API int beginSlitPlugin (HINSTANCE hMainInstance, HWND hwndBBSlit);
	BBANALOG_API void endPlugin (HINSTANCE hMainInstance);
	BBANALOG_API LPCSTR pluginInfo (int field);
}

#include "bblib.h"
#include "bbversion.h"
#include <time.h>
#include <math.h>

#define IDT_TIMER 2

#ifndef M_PI
#define M_PI 3.1415926536
#endif

//===========================================================================

const char szVersion	  [] = "bbAnalog 1.0 AA";  // Used in MessageBox titlebars
const char szAppName	  [] = "bbAnalog";		   // The name of our window class, etc.
const char szInfoVersion  [] = "1.0 AA";
const char szInfoAuthor   [] = "mortar, grischka";
const char szInfoRelDate  [] = BB_RELDATE;
const char szInfoLink	  [] = "http://bb4win.sourceforge.net/bblean";
const char szInfoEmail	  [] = "grischka@users.sourceforge.net";
const char szCopyright	  [] = "2004-2009";

LPCSTR pluginInfo(int field)
{
	switch (field)
	{
		default:
		case 0: return szVersion;
		case 1: return szAppName;
		case 2: return szInfoVersion;
		case 3: return szInfoAuthor;
		case 4: return szInfoRelDate;
		case 5: return szInfoLink;
		case 6: return szInfoEmail;
	}
}

const wchar_t szVersionW[] = L"bbAnalog 1.0 AA";  // Used in MessageBox titlebars
const wchar_t szAppNameW[] = L"bbAnalog";		  // The name of our window class, etc.
const wchar_t szInfoVersionW[] = L"1.0 AA";
const wchar_t szInfoAuthorW[] = L"mortar, grischka";
const wchar_t szInfoRelDateW[] = TEXT(BB_RELDATE);
const wchar_t szInfoLinkW[] = L"http://bb4win.sourceforge.net/bblean";
const wchar_t szInfoEmailW[] = L"grischka@users.sourceforge.net";
const wchar_t szCopyrightW[] = L"2004-2009";

wchar_t const * pluginInfoW(int field)
{
	switch (field)
	{
		default:
		case 0: return szVersionW;
		case 1: return szAppNameW;
		case 2: return szInfoVersionW;
		case 3: return szInfoAuthorW;
		case 4: return szInfoRelDateW;
		case 5: return szInfoLinkW;
		case 6: return szInfoEmailW;
	}
}

//===========================================================================
// global variables, valid for all instances

HWND BBhwnd;

StyleItem myStyleItem;
StyleItem myStyleItem2;
int bevelWidth;
int borderWidth;
COLORREF borderColor;

void GetStyleSettings();

struct plugin_info * g_PI;

//===========================================================================

struct bbanalog_plugin : plugin_info
{
	wchar_t instance_key[40];

	wchar_t date_format[96];
	SYSTEMTIME clocktime;
	int centerX;
	int centerY;

	int showDate;
	bool antiAliasing;
	bool drawBorder;

	void ReadRCSettings ();
	void getCurrentTime ();
	void DrawRadian (HDC hdc, int rfrom, int rto, int seconds, COLORREF Colour, int thickness);
	void draw_clock (HDC);

	LRESULT wnd_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT * ret);
	void process_broam (const char *temp, int f);
	void show_menu (bool);

	void about_box ()
	{
		BBP_messagebox(this, MB_OK, "%s\n%s\n© %s %s", szVersion, "© 2003 Mortar - Brad Bartolucci", szCopyright, szInfoEmail);
	}

	bbanalog_plugin ()
		: plugin_info()
		, centerX(0)
		, centerY(0)
		, showDate(0)
		, antiAliasing(false)
		, drawBorder(false)
	{
		instance_key[0] = '\0';
//		  BBP_clear(this, FIRSTITEM);

		this->next = g_PI;
		g_PI = this;

		BBP_read_window_modes(this, szAppNameW);

// 				if (0 == rcpath[0])
// 					BBP_get_rcpath(rcpath, hInstance, rcfile);

				xpos = 20;
				ypos = 20;

				place = 0;

				hMon = GetMonitorRect(&xpos, &mon_rect, GETMON_FROM_POINT);
				//set_place(PI);

				useSlit = false;
				alwaysOnTop = false;
				autoHide = false;
				snapWindow = 20;
				pluginToggle = true;
				clickRaise = true;
				alphaEnabled = false;
				alphaValue = 192;
				//orient_vertical = is_bar || 0 == _stricmp("vertical", BBP_read_string(PI, NULL, "orientation", "vertical"));
				//if (false == no_icons)
				{
					saturation = 80;
					hue = 60;
				}

				width = height = 48;
				antiAliasing = true;
				drawBorder = true;
				_tcscpy(date_format, L"%a|%#d|%b");
	}

	~bbanalog_plugin ()
	{
		BBP_Exit_Plugin(this);
		for (plugin_info ** pp = &g_PI; *pp; pp = &(*pp)->next)
		{
			if (this == *pp)
			{
				*pp = this->next;
				break;
			}
		}
	}
};

//===========================================================================

plugin_info * start_plugin (HINSTANCE hPluginInstance, HWND hSlit)
{
	plugin_info * p = nullptr;
	bbanalog_plugin * PI = nullptr;
	int n = 0;

	if (NULL == BBhwnd)
	{
		// this is done only when the first instance is loaded
		BBhwnd = GetBBWnd();
		GetStyleSettings();
	}

	// count instances so far:
	for (p = g_PI, n = 0; p; p = p->next, ++n)
		;

	// create it:
	PI = new bbanalog_plugin;

	// make an unique identifier
	_snwprintf(PI->instance_key, 40, n == 0 ? L"%s" : L"%s.%d", szAppNameW, n + 1);

	// setup stuff
	PI->hSlit		= hSlit;
	PI->hInstance	= hPluginInstance;
	PI->class_name	= szAppNameW;
	_tcsncpy(PI->rc_key, PI->instance_key, 40);
	_tcsncpy(PI->broam_key, PI->instance_key, 40);

	// Get plugin settings...
	PI->ReadRCSettings();

	// create the plugin window
	if (0 == BBP_Init_Plugin(PI))
	{
		delete PI;
		return NULL;
	}

	return PI;
}

int beginPluginEx(HINSTANCE hPluginInstance, HWND hSlit)
{
	if (NULL == start_plugin(hPluginInstance, hSlit))
		return BEGINPLUGIN_FAILED;
	return BEGINPLUGIN_OK;
}

void endPlugin(HINSTANCE hPluginInstance)
{
	if (g_PI)
		delete g_PI;
}

//===========================================================================

//===========================================================================
// draw antialiased line

void BlendPixel(HDC hdc, int x, int y, COLORREF Paint, BYTE Alpha)
{
	COLORREF Paper = GetPixel(hdc, x, y);

	BYTE PixelAlpha = Alpha;// * 5 / 10;
	BYTE PaperAlpha = ~PixelAlpha;
	SetPixel(hdc, x, y,
		(((((Paint&0x0000FF)*PixelAlpha)+((Paper&0x0000FF)*PaperAlpha))&0x0000FF00)
	   | ((((Paint&0x00FF00)*PixelAlpha)+((Paper&0x00FF00)*PaperAlpha))&0x00FF0000)
	   | ((((Paint&0xFF0000)*PixelAlpha)+((Paper&0xFF0000)*PaperAlpha))&0xFF000000)
		 ) >> 8);
}

void DrawAALine(HDC hdc, int x0, int y0, int x1, int y1, DWORD Colour, int thickness)
{
	int dx, dy, xDir, t;
	if(y0>y1)
	{
	  t = y0, y0 = y1, y1=t; //Swap(y0,y1);
	  t = x0, x0 = x1, x1=t; //Swap(x0,x1);
	}

	//First and last Pixels always get set
	SetPixel(hdc, x0, y0, Colour);
	SetPixel(hdc, x1, y1, Colour);

	dx=x1-x0;
	dy=y1-y0;

	if(dx>=0)	xDir=1;
	else		xDir=-1, dx=-dx;

	DWORD ErrorAcc = 0;
	BYTE Alpha;

	if(dy>dx) {
		// y-major line
		DWORD ErrorAdj = ((DWORD)dx<<16) / (DWORD)dy;
		while(--dy) {
			ErrorAcc+=ErrorAdj;
			++y0;
			x1=x0+xDir*(WORD)(ErrorAcc>>16);
			Alpha=(BYTE)(ErrorAcc>>8);
			BlendPixel(hdc, x1,		 y0, Colour, ~Alpha);
			BlendPixel(hdc, x1+xDir, y0, Colour,  Alpha);
		}

	} else {
		// x-major line
		DWORD ErrorAdj=((DWORD)dy<<16) / (DWORD)dx;
		while(--dx) {
			ErrorAcc+=ErrorAdj;
			x0+=xDir;
			y1=y0+(WORD)(ErrorAcc>>16);
			Alpha=(BYTE)(ErrorAcc>>8);
			BlendPixel(hdc, x0, y1	, Colour, ~Alpha);
			BlendPixel(hdc, x0, y1+1, Colour,  Alpha);
		}
	}
}

//===========================================================================
// draw not-antialiased line

void DrawLine(HDC hdc, int x0, int y0, int x1, int y1, DWORD Colour, int thickness)
{
	HGDIOBJ hpen = SelectObject(hdc, CreatePen(PS_SOLID, 1, Colour));
	MoveToEx(hdc, x0, y0, NULL);
	LineTo(hdc, x1, y1);
	DeleteObject(SelectObject(hdc, hpen));
	SetPixel(hdc, x1, y1, Colour);
}

//===========================================================================
// Draw the clock hands

void bbanalog_plugin::DrawRadian (HDC hdc, int rfrom, int rto, int seconds, COLORREF Colour, int thickness)
{
	double theta = seconds * (2*M_PI) / 3600;

	double fy = cos(theta);
	double fx = sin(theta);
	int px = (int)(centerX + (rfrom * fx) + 0.5);
	int py = (int)(centerY - (rfrom * fy) + 0.5);

	if (rto == rfrom) {
		SetPixel(hdc, px, py, Colour);

	} else {
		int qx = (int)(centerX + (rto * fx) + 0.5);
		int qy = (int)(centerY - (rto * fy) + 0.5);
		if (antiAliasing)
			DrawAALine(hdc, px, py, qx, qy, Colour, thickness);
		else
			DrawLine(hdc, px, py, qx, qy, Colour, thickness);
	}
}

//===========================================================================
// draw the clock

void bbanalog_plugin::draw_clock (HDC hdc)
{
	COLORREF fontColor = myStyleItem.TextColor;
	RECT r = { 0, 0, this->width, this->height};

	//Make background gradient
	MakeStyleGradient(hdc, &r, &myStyleItem, drawBorder);

	if (false == myStyleItem2.parentRelative)
	{
		int indent = bevelWidth + borderWidth;
		r.left		+= indent;
		r.top		+= indent;
		r.bottom	-= indent;
		r.right		-= indent;

		MakeStyleGradient(hdc, &r, &myStyleItem2, false);
	}


	if(showDate)
	{
		HGDIOBJ otherfont = SelectObject(hdc, CreateStyleFont(&myStyleItem));

		time_t systemTime; time(&systemTime);
		struct tm *ltm = localtime(&systemTime);

		wchar_t currentDate[80], *p;
		_tcsftime(currentDate, sizeof currentDate, date_format, ltm);

		for (p = currentDate; *p; ++p)
			if (*p == '|')
				*p = '\n';

		RECT s = {0,0,0,0};
		DrawText(hdc, currentDate, -1, &s, DT_LEFT|DT_CALCRECT);
		r.top += (r.bottom - r.top - s.bottom) / 2;

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, fontColor);
		DrawText(hdc, currentDate, -1, &r, DT_TOP | DT_CENTER );
		DeleteObject(SelectObject(hdc, otherfont));
	}
	else
	{
		int i, radius;

		radius = (this->width / 2) - (bevelWidth + borderWidth) - 1;
		centerX = this->width  / 2;
		centerY = this->height / 2;

		// Draw tics(1-12) on the clock face.
		// 12, 3, 6, 9 are 3 pixels long, the rest are 1 pixel.
		for(i = 0; i<12; i++)
			DrawRadian (hdc,
				radius-1-2*(0==i%3),
				radius-1,
				i * (3600 / 12),
				fontColor,
				10);

		//Draw the seconds hand
		DrawRadian (hdc,
			-radius/5,
			radius-3,
			clocktime.wSecond * (3600 / 60),
			0x0066bb, // dark red
			10);

		//Draw the minute hand
		DrawRadian (hdc,
			2,
			radius-4,
			clocktime.wMinute * (3600 / 60),
			fontColor,
			10);

		//Draw the hour hand
		DrawRadian (hdc,
			2,
			(radius - 3)*2/3,
			clocktime.wHour * (3600 / 12) + clocktime.wMinute * (3600 / 60 / 12),
			fontColor,
			15);

		// Set a center pixel
		SetPixel(hdc, centerX, centerY, fontColor);
	}
}

//===========================================================================

void bbanalog_plugin::process_broam(const char *temp, int f)
{
//	   int size;
//	   const char *rest;
// 
//	   if (f) // the message has already been handled by bbPlugin
//	   {
//		   show_menu(false);
//		   return;
//	   }
// 
//	   if (BBP_broam_bool(this, temp, "antiAliasing", &antiAliasing))
//	   {
//		   InvalidateRect(this->hwnd, NULL, FALSE);
//		   show_menu(false);
//		   return;
//	   }
// 
//	   if (BBP_broam_bool(this, temp, "drawBorder", &drawBorder))
//	   {
//		   InvalidateRect(this->hwnd, NULL, FALSE);
//		   show_menu(false);
//		   return;
//	   }
// 
//	   if (BBP_broam_int(this, temp, "Size", &size))
//	   {
//		   BBP_set_size(this, size, size);
//		   show_menu(false);
//		   return;
//	   }
// 
//	   if (BBP_broam_string(this, temp, "dateFormat", &rest))
//	   {
//		   _tcscpy(date_format, rest);
//		   InvalidateRect(this->hwnd, NULL, FALSE);
//		   show_menu(false);
//		   return;
//	   }
}

//===========================================================================
LRESULT bbanalog_plugin::wnd_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *ret)
{

	if (ret) // the message has already been handled by bbPlugin
	{
		return *ret;
	}

	switch (message)
	{
		case WM_CREATE:
			SetTimer(hwnd, IDT_TIMER, 1000, (TIMERPROC) NULL);
			this->getCurrentTime();
			break;

		case WM_DESTROY:
			break;

		case WM_PAINT:
		{
			// Create buffer hdc's, bitmaps etc.
			PAINTSTRUCT ps;
			HDC hdc_scrn = BeginPaint(hwnd, &ps);
			HDC buf = CreateCompatibleDC(NULL);
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc_scrn, this->width, this->height);
			HGDIOBJ otherbmp = SelectObject(buf, bufbmp);

			this->draw_clock(buf);

			BitBltRect(hdc_scrn, buf, &ps.rcPaint);
			DeleteObject(SelectObject(buf, otherbmp));
			DeleteDC(buf);

			EndPaint(hwnd, &ps);
			break;
		}

		case BB_RECONFIGURE:
//			   GetStyleSettings();
//			   this->ReadRCSettings();
//			   this->getCurrentTime();
//			   BBP_reconfigure(this);
			break;

		//Show the date when the mouse is clicked
		case WM_LBUTTONUP:
			this->showDate = this->showDate ? 0 : 3;
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case WM_LBUTTONDBLCLK:
			//open control panel
//			  BBExecute(NULL, NULL, "control", "timedate.cpl,system,0", NULL, SW_SHOWNORMAL, false);
			break;

		// Right mouse button clicked?
		case WM_RBUTTONUP:
			this->show_menu (true);
			break;

		case WM_TIMER:
			if (IDT_TIMER == wParam)
			{
				this->getCurrentTime();
				SetTimer(hwnd, IDT_TIMER, 1100 - this->clocktime.wMilliseconds, NULL);
				if (this->showDate)
					--this->showDate;
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			}
			break;

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

//===========================================================================
void bbanalog_plugin::show_menu (bool pop)
{
//	   n_menu *myMenu = n_makemenu(this->instance_key);
//	   n_menu *cm  = n_submenu(myMenu, "Configuration");
//	   n_menuitem_int(cm, "Size", "Size", this->width, 16, 240);
//	   n_menuitem_str(cm, "Date Format", "dateFormat", date_format);
//	   n_menuitem_bol(cm, "AntiAliasing", "antiAliasing", antiAliasing);
//	   n_menuitem_bol(cm, "Draw Border", "drawBorder", drawBorder);
//	   n_menuitem_nop(cm);
//	   BBP_n_insertmenu(this, cm);
//	   if (false == this->inSlit) {
//		   if (this->autoHide) {
//			   n_menuitem_nop(cm);
//			   BBP_n_orientmenu(this, cm);
//		   }
//		   BBP_n_placementmenu(this, myMenu);
//	   }
//	   n_menuitem_cmd(myMenu, "Edit Settings", "EditRC");
//	   n_menuitem_cmd(myMenu, "About", "About");
//	   n_showmenu(this, myMenu, pop, 0);
}

//===========================================================================

void GetStyleSettings()
{
	myStyleItem = *(StyleItem *)GetSettingPtr(SN_TOOLBAR);
	myStyleItem2 = *(StyleItem *)GetSettingPtr(SN_TOOLBARLABEL);
	bevelWidth = *(int*)GetSettingPtr(SN_BEVELWIDTH);
	borderWidth = *(int*)GetSettingPtr(SN_BORDERWIDTH);
	borderColor = *(COLORREF*)GetSettingPtr(SN_BORDERCOLOR);
}

//===========================================================================

void bbanalog_plugin::ReadRCSettings()
{
	BBP_read_window_modes(this, szAppNameW);

	int sz = BBP_read_int(this, L"size", 48);
	this->width = this->height = imax(sz, 16);
	antiAliasing = BBP_read_bool(this, L"antiAliasing", true);
	drawBorder = BBP_read_bool(this, L"drawBorder", true);
	BBP_read_string(this, date_format, sizeof(date_format)/sizeof(*date_format), L"dateFormat", L"%a|%#d|%b");
}

//===========================================================================

void bbanalog_plugin::getCurrentTime()
{
	// current local time of the users machine
	GetLocalTime(&clocktime);
}

//===========================================================================
