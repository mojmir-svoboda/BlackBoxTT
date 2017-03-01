#pragma once
#include <bblib/platform_win.h>
#include <bblib/bbassert.h>
#include "tinylist.h"
//#include "winutils.h"
//#include "numbers.h"
#include "memory.h"
#include "utils_string.h"
#include "colors.h"
#include "styleprops.h"
#include "StyleItem.h"
#define API_EXPORT extern

#define MAX_LINE_LENGTH 1024

#define V_MAR 0x0200
/* BlackboxZero 1.4.2012 */
#define V_SHADOWX		0x2000
#define V_SHADOWY		0x4000
#define V_SHADOWCOLOR	0x8000
#define V_OUTLINECOLOR	0x10000
#define V_FROMSPLITTO	0x20000
#define	V_TOSPLITTO		0x40000

#define V_SHADOW (V_SHADOWX|V_SHADOWY|V_SHADOWCOLOR)
#define V_SPLIT (V_FROMSPLITTO|V_TOSPLITTO)

API_EXPORT const wchar_t * GetBBVersion ();
/* Draw a Gradient Rectangle from StyleItem, optional using the style border. */
API_EXPORT void MakeStyleGradient (HDC hDC, RECT const * p_rect, StyleItem const * m_si, bool withBorder);
/* Draw a Border */
API_EXPORT void CreateBorder (HDC hdc, RECT const * p_rect, COLORREF borderColour, int borderWidth);
/* Draw a Pixmap for buttons, menu bullets, checkmarks ... */
API_EXPORT void bbDrawPix (HDC hDC, RECT *p_rect, COLORREF picColor, int style);
/* Create a font handle from styleitem, with parsing and substitution. */
API_EXPORT HFONT CreateStyleFont (StyleItem const * si);

API_EXPORT int BBDrawTextAltW (HDC hDC, LPCWSTR lpString, int nCount, RECT * lpRect, unsigned uFormat, StyleItem * si);

//====================
// Plugin Snap
API_EXPORT int Settings_snapThreshold;
API_EXPORT int Settings_snapPadding;
API_EXPORT bool Settings_snapPlugins;

//===========================================================================
// API: SnapWindowToEdge
// Purpose:Snaps a given windowpos at a specified distance
// In: WINDOWPOS* = WINDOWPOS recieved from WM_WINDOWPOSCHANGING
// In: int = distance to snap to
// In: bool = use screensize of workspace
//===========================================================================

/* Public flags for SnapWindowToEdge */
#define SNAP_FULLSCREEN 1  /* use full screen rather than workarea */
#define SNAP_NOPLUGINS  2 /* dont snap to other plugins */
#define SNAP_SIZING     4 /* window is resized (bottom-right sizing only) */
/* Private flags for SnapWindowToEdge */
#define SNAP_NOPARENT   8  /* dont snap to parent window edges */
#define SNAP_NOCHILDS  16  /* dont snap to child windows */
#define SNAP_TRIGGER   32  /* apply nTriggerDist instead of default */
#define SNAP_PADDING   64  /* Next arg points to the padding value */
#define SNAP_CONTENT  128  /* Next arg points to a SIZE struct */
API_EXPORT void SnapWindowToEdge(WINDOWPOS* wp, LPARAM nTriggerDist, UINT Flags, ...);

/* Window utilities */
/* GetMonitorRect Flags: */
#define GETMON_FROM_WINDOW    1 /* 'from' is HWND */
#define GETMON_FROM_POINT     2 /* 'from' is POINT* */
#define GETMON_FROM_MONITOR   4 /* 'from' is HMONITOR */
#define GETMON_WORKAREA      16 /* get working area rather than full screen */
API_EXPORT HMONITOR GetMonitorRect(void *from, RECT *r, int Flags);


/* Wrapper for 'SetLayeredWindowAttributes', win9x compatible */
/* alpha: 0-255, 255=transparency off */
API_EXPORT bool SetTransparency(HWND hwnd, BYTE alpha);



/* Read Settings */
// API_EXPORT bool ReadBool(const char* fileName, const char* szKey, bool defaultBool);
// API_EXPORT int ReadInt(const char* fileName, const char* szKey, int defaultInt);
// API_EXPORT COLORREF ReadColor(const char* fileName, const char* szKey, const char* defaultColor);
// API_EXPORT const char* ReadString(const char* fileName, const char* szKey, const char* defaultString);
API_EXPORT bool ReadBool (const wchar_t * fileName, const wchar_t * szKey, bool defaultBool);
API_EXPORT int ReadInt (const wchar_t * fileName, const wchar_t * szKey, int defaultInt);
API_EXPORT COLORREF ReadColor (const wchar_t * fileName, const wchar_t * szKey, const wchar_t * defaultColor);
API_EXPORT const wchar_t * ReadString (const wchar_t * fileName, const wchar_t * szKey, const wchar_t * defaultString);


/* Read a rc-value as string. 'ptr' is optional, if present, at input indicates the line
from where the search starts, at output is set to the line that follows the match. */
//API_EXPORT const char* ReadValue(const char* fileName, const char* szKey, long* ptr = nullptr);
API_EXPORT const wchar_t * ReadValue(const wchar_t * fileName, const wchar_t * szKey, long* ptr = nullptr);

/* Note that pointers returned from 'ReadString' and 'ReadValue' are valid only
until the next Read/Write call. For later usage, you need to copy the string
into a place within your code. */

API_EXPORT int FoundLastValue(void);
/* Returns: 0=not found, 1=found exact value, 2=found matching wildcard */

/* Write Settings */
API_EXPORT void WriteBool (const wchar_t * fileName, const wchar_t * szKey, bool value);
API_EXPORT void WriteInt (const wchar_t * fileName, const wchar_t * szKey, int value);
API_EXPORT void WriteString (const wchar_t * fileName, const wchar_t * szKey, const wchar_t * value);
API_EXPORT void WriteColor (const wchar_t * fileName, const wchar_t * szKey, COLORREF value);

/* ------------------------------------ */
/* Shell execute a command */
API_EXPORT BOOL BBExecute (
	HWND Owner,         /*  normally NULL */
	const wchar_t * szVerb,      /*  normally NULL */
	const wchar_t * szFile,      /*  required */
	const wchar_t * szArgs,      /*  or NULL */
	const wchar_t * szDirectory, /*  or NULL */
	int nShowCmd,       /*  normally SW_SHOWNORMAL */
	int noErrorMsgs    /*  if true, suppresses errors */
);
API_EXPORT int BBMessageBox (int flg, const wchar_t *fmt, ...);

/* Put first 'numTokens' from 'source' into 'targets', copy rest into
'remaining', if it's not NULL. The 'targets' and 'remaining' buffers
are zero-terminated always. Returns the number of actual tokens found */
API_EXPORT 
int BBTokenize (
	const wchar_t * src,
	wchar_t ** buffs,
	size_t * buff_sizes,
	unsigned buff_count,
	wchar_t * rest_of_string,
	size_t rest_of_string_size,
	bool unquote
);

API_EXPORT bool BBExecute_string (const wchar_t * s, int flags);

