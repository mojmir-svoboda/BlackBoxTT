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
API_EXPORT int BBTokenize ( const wchar_t* source, wchar_t** targets, unsigned numTokens, wchar_t* remaining);

API_EXPORT bool BBExecute_string (const wchar_t * s, int flags);

/* ------------------------------------ */
/* Plugin Menu API - See the SDK for application examples */

/* creates a Menu or Submenu, Id must be unique, fshow indicates whether
the menu should be shown (true) or redrawn (false) */
// API_EXPORT Menu *MakeNamedMenu(const char* HeaderText, const char* Id, bool fshow);
// API_EXPORT MenuItem * MakeMenuGrip (Menu * PluginMenu, LPCSTR Title ISNULL);
// 
// /* inserts an item to execute a command or to set a boolean value */
// API_EXPORT MenuItem *MakeMenuItem(
// 	Menu *PluginMenu, const char* Title, const char* Cmd, bool ShowIndicator);
// 
// /* inserts an inactive item, optionally with text. 'Title' may be NULL. */
// API_EXPORT MenuItem *MakeMenuNOP(Menu *PluginMenu, const char* Title ISNULL);
// 
// /* inserts an item to adjust a numeric value */
// API_EXPORT MenuItem *MakeMenuItemInt(
// 	Menu *PluginMenu, const char* Title, const char* Cmd,
// 	int val, int minval, int maxval);
// 
// /* inserts an item to edit a string value */
// API_EXPORT MenuItem *MakeMenuItemString(
// 	Menu *PluginMenu, const char* Title, const char* Cmd,
// 	const char* init_string ISNULL);
// 
// /* inserts an item, which opens a submenu */
// API_EXPORT MenuItem *MakeSubmenu(
// 	Menu *ParentMenu, Menu *ChildMenu, const char* Title ISNULL);
// 
// /* inserts an item, which opens a submenu from a system folder.
// 'Cmd' optionally may be a Broam which then is sent on user click
// with "%s" in the broam string replaced by the selected filename */
// API_EXPORT MenuItem* MakeMenuItemPath(
// 	Menu *ParentMenu, const char* Title, const char* path, const char* Cmd ISNULL);
// 
// /* Context menu for filesystem items. One of path or pidl can be NULL */
// API_EXPORT Menu *MakeContextMenu(const char *path, const void *pidl);
// 
// /* shows the menu */
// API_EXPORT void ShowMenu(Menu *PluginMenu);
// 
// /* checks whether a menu with ID starting with 'IDString_start', still exists */
// API_EXPORT bool MenuExists(const char* IDString_start);
// 
// /* set option for MenuItem  */
// API_EXPORT void MenuItemOption(MenuItem *pItem, int option, ...);
// #define BBMENUITEM_DISABLED   1 /* set disabled state */
// #define BBMENUITEM_CHECKED    2 /* set checked state */
// #define BBMENUITEM_LCOMMAND   3 /* next arg is command for left click */
// #define BBMENUITEM_RCOMMAND   4 /* next arg is command for right click */
// #define BBMENUITEM_OFFVAL     5 /* next args are offval, offstring (with Int-Items) */
// #define BBMENUITEM_UPDCHECK   6 /* update checkmarks on the fly */
// #define BBMENUITEM_JUSTIFY    7 /* next arg is DT_LEFT etc... */
// #define BBMENUITEM_SETICON    8 /* next arg is "path\to\icon[,iconindex]" */
// #define BBMENUITEM_SETHICON   9 /* next arg is HICON */
// #define BBMENUITEM_RMENU     10 /* next arg is Menu* for right-click menu */
// 
// API_EXPORT void MenuOption(Menu *pMenu, int flags, ...);
// #define BBMENU_XY             0x0001 /* next arg is x/y position */
// #define BBMENU_RECT           0x0002 /* next arg is *pRect to show above/below */
// #define BBMENU_CENTER         0x0003 /* center menu on screen */
// #define BBMENU_CORNER         0x0004 /* align with corner on mouse */
// #define BBMENU_POSMASK        0x0007 /* bit mask for above positions */
// #define BBMENU_KBD            0x0008 /* use position from blackbox.rc */
// #define BBMENU_XRIGHT         0x0010 /* x is menu's right */
// #define BBMENU_YBOTTOM        0x0020 /* y is menu's bottom */
// #define BBMENU_PINNED         0x0040 /* show menu initially pinned */
// #define BBMENU_ONTOP          0x0080 /* show menu initially on top */
// #define BBMENU_NOFOCUS        0x0100 /* dont set focus on menu */
// #define BBMENU_NOTITLE        0x0200 /* no title */
// #define BBMENU_MAXWIDTH       0x0400 /* next arg is maximal menu width */
// #define BBMENU_SORT           0x0800 /* sort menu alphabetically */
// #define BBMENU_ISDROPTARGET   0x1000 /* register as droptarget */
// #define BBMENU_HWND           0x2000 /* next arg is HWND to send notification on menu-close */
// #define BBMENU_SYSMENU        0x4000 /* is a system menu (for bbLeanSkin/Bar) */
