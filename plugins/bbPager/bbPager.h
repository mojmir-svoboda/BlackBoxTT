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
#pragma once
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define TME_NONCLIENT		0x00000010

#include "BBApi.h"
#include <commctrl.h>
#include <shellapi.h>
#include <vector>

//const long magicDWord = 0x49474541;


// data structures
struct FRAME
{
	int width;
	int height;
	int rows;
	int columns;
	int bevelWidth;
	int borderWidth;
	int hideWidth;

	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	bool ownStyle;

	StyleItem *style;
	StyleItem Style;

	FRAME () { memset(this, 0, sizeof(*this)); }
};

struct DESKTOP
{
	int width;
	int height;
	int sizeRatio;

	int fontSize;
	int fontWeight;

	char fontFace[64];

	bool numbers;
	bool windows;
	bool tooltips;

	COLORREF fontColor;
	COLORREF color;
	COLORREF colorTo;

	bool ownStyle;

	StyleItem *style;
	StyleItem Style;

	DESKTOP () { memset(this, 0, sizeof(*this)); }
};

struct ACTIVEDESKTOP
{
	char styleType[16];

	//bool useDesktopStyle;

	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	bool ownStyle;

	StyleItem *style;
	StyleItem Style;

	ACTIVEDESKTOP () { memset(this, 0, sizeof(*this)); }
};

struct WINDOW
{
	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	bool ownStyle;

	StyleItem *style;
	StyleItem Style;

	WINDOW () { memset(this, 0, sizeof(*this)); }
};

struct FOCUSEDWINDOW
{
	char styleType[16];

	//bool useWindowStyle;

	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	bool ownStyle;

	StyleItem *style;
	StyleItem Style;

	FOCUSEDWINDOW () { memset(this, 0, sizeof(*this)); }
};

struct POSITION
{
	int x;
	int y;
	int ox;
	int oy;
	int hx;
	int hy;
	int side;

	bool vertical;
	bool horizontal;
	bool grid;
	bool raised;
	int snapWindow;
	bool unix;
	bool autohide;
	bool autohideOld;
	bool hidden;

	char placement[20];

	POSITION () { memset(this, 0, sizeof(*this)); }
};

struct WinStruct
{
	HWND window;
	RECT r;
	BOOL active;
	BOOL sticky;
	int desk;

	WinStruct () { memset(this, 0, sizeof(*this)); }
};

// flashing tasks
struct FlashTask
{
	HWND task;
	bool on;
};

struct RuntimeState
{
	HINSTANCE m_hInstance;
	HWND m_hwndBBPager;
	HWND m_hwndBlackbox;
	int m_currentDesktop;
	int m_desktops;
	int m_desktopsY;
	int m_winCount;
	bool m_usingAltMethod;
	bool m_is_xoblite;
	bool m_winMoving;
	bool m_usingWin2kXP;
	std::vector<WinStruct> m_winList;
	WinStruct m_moveWin;

	RuntimeState ()
		: m_hInstance(0)
		, m_hwndBBPager(0)
		, m_hwndBlackbox(0)
		, m_currentDesktop(0)
		, m_desktops(-1)
		, m_desktopsY(-1)
		, m_winCount(0)
		, m_usingAltMethod(false)
		, m_is_xoblite(false)
		, m_winMoving(false)
		, m_usingWin2kXP(false)
		, m_winList()
		, m_moveWin()
	{ }
};

RuntimeState & getRuntimeState ();

//===========================================================================
// function declarations

LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CheckTaskEnumProc (HWND hwnd, LPARAM lParam);
BOOL CALLBACK CheckTaskEnumProc_AltMethod (HWND hwnd, LPARAM lParam);

void DrawBBPager (HWND hwnd);
void DrawBorder (HDC hdc, RECT rect, COLORREF borderColour, int borderWidth);

void GetPos (bool snap);
void SetPos (int place);

bool IsValidWindow (HWND hWnd);
int getDesktop (HWND h);

void UpdatePosition ();
void UpdateMonitorInfo ();

void ClickMouse ();
void TrackMouse ();
bool CursorOutside ();

void DeskSwitch ();

void FocusWindow ();
void GrabWindow ();
void DropWindow ();

void AddFlash (HWND task);
void RemoveFlash (HWND task, bool quick);
bool IsFlashOn (HWND task);

void SetToolTip (RECT * tipRect, char * tipText);
void ClearToolTips ();

void HidePager ();

void DisplayMenu ();

void ToggleSlit ();

bool AddBBWindow(tasklist const * tl);

bool BBPager_SetTaskLocation (HWND hwnd, taskinfo const *pti, UINT flags);
tasklist const * BBPager_GetTaskListPtr(void);

//===========================================================================

