/* ==========================================================================
	This file is part of the bbLean source code
	Copyright © 2001-2003 The Blackbox for Windows Development Team
	Copyright © 2004-2009 grischka
	========================================================================== */
#pragma once
#include "StyleItem.h"

/*=========================================================================== */
/* constants for GetSettingPtr(int index) -> returns: */
enum SN_INDEX : int
{
	SN_STYLESTRUCT = 0, /* StyleStruct* */

	SN_TOOLBAR = 1, /* StyleItem* */
	SN_TOOLBARBUTTON, /* StyleItem* */
	SN_TOOLBARBUTTONP, /* StyleItem* */
	SN_TOOLBARLABEL, /* StyleItem* */
	SN_TOOLBARWINDOWLABEL, /* StyleItem* */
	SN_TOOLBARCLOCK, /* StyleItem* */
	SN_MENUTITLE, /* StyleItem* */
	SN_MENUFRAME, /* StyleItem* */
	SN_MENUHILITE, /* StyleItem* */

	SN_MENUBULLET, /* char* */
	SN_MENUBULLETPOS              /* char* */
	,
	SN_BORDERWIDTH, /* int* */
	SN_BORDERCOLOR, /* COLORREF* */
	SN_BEVELWIDTH, /* int* */
	SN_FRAMEWIDTH, /* int* */
	SN_HANDLEHEIGHT, /* int* */
	SN_ROOTCOMMAND, /* char* */

	SN_MENUALPHA, /* int* */
	SN_TOOLBARALPHA, /* int* */
	SN_METRICSUNIX, /* bool* */
	SN_BULLETUNIX, /* bool* */

	SN_WINFOCUS_TITLE, /* StyleItem* */
	SN_WINFOCUS_LABEL, /* StyleItem* */
	SN_WINFOCUS_HANDLE, /* StyleItem* */
	SN_WINFOCUS_GRIP, /* StyleItem* */
	SN_WINFOCUS_BUTTON, /* StyleItem* */
	SN_WINFOCUS_BUTTONP           /* StyleItem* */
	,
	SN_WINUNFOCUS_TITLE, /* StyleItem* */
	SN_WINUNFOCUS_LABEL, /* StyleItem* */
	SN_WINUNFOCUS_HANDLE, /* StyleItem* */
	SN_WINUNFOCUS_GRIP, /* StyleItem* */
	SN_WINUNFOCUS_BUTTON, /* StyleItem* */

	SN_WINFOCUS_FRAME_COLOR, /* COLORREF* */
	SN_WINUNFOCUS_FRAME_COLOR, /* COLORREF* */

	SN_NEWMETRICS, /* bool (not a ptr) */

	SN_ISSTYLE070, /* bool* */
	SN_SLIT, /* StyleItem* */

	SN_MENUSEPMARGIN, /* int* */
	SN_MENUSEPCOLOR, /* COLORREF* */
	SN_MENUSEPSHADOWCOLOR, /* COLORREF* */
	SN_MENUGRIP, /* StyleItem* */

	SN_LAST
};

/* Note: Do not change this structure. New items may be appended at the end, though. */

struct StyleStruct
{
	StyleItem Toolbar;
	StyleItem ToolbarButton;
	StyleItem ToolbarButtonPressed;
	StyleItem ToolbarLabel;
	StyleItem ToolbarWindowLabel;
	StyleItem ToolbarClock;

	StyleItem MenuTitle;
	StyleItem MenuFrame;
	StyleItem MenuHilite;

	StyleItem windowTitleFocus;
	StyleItem windowLabelFocus;
	StyleItem windowHandleFocus;
	StyleItem windowGripFocus;
	StyleItem windowButtonFocus;
	StyleItem windowButtonPressed;

	StyleItem windowTitleUnfocus;
	StyleItem windowLabelUnfocus;
	StyleItem windowHandleUnfocus;
	StyleItem windowGripUnfocus;
	StyleItem windowButtonUnfocus;

	COLORREF windowFrameFocusColor;
	COLORREF windowFrameUnfocusColor;

	unsigned char menuAlpha;
	unsigned char toolbarAlpha;
	bool menuNoTitle;
	unsigned char reserved2;

	COLORREF borderColor;
	int borderWidth;
	int bevelWidth;
	int frameWidth;
	int handleHeight;

	char menuBullet[16];
	char menuBulletPosition[16];
	char rootCommand[MAX_PATH+80];

	bool bulletUnix;
	bool metricsUnix;
	bool is_070;
	bool menuTitleLabel;
	bool nix;

	StyleItem Slit;

	/* BlackboxZero 1.8.2012
	** These will break anything expecting to acquire the whole structure
	** and not built against this base */
	int MenuSepMargin;
	COLORREF MenuSepColor;
	COLORREF MenuSepShadowColor;
	StyleItem MenuGrip;

	wchar_t menuBulletW[16];
	wchar_t menuBulletPositionW[16];
	wchar_t rootCommandW[MAX_PATH + 80];

	//===========================================================================
	void * GetStyleMemberPtr (int sn_index)
	{
		switch (sn_index) {
		case SN_STYLESTRUCT: return this;

		case SN_TOOLBAR: return &Toolbar;
		case SN_TOOLBARBUTTON: return &ToolbarButton;
		case SN_TOOLBARBUTTONP: return &ToolbarButtonPressed;
		case SN_TOOLBARLABEL: return &ToolbarLabel;
		case SN_TOOLBARWINDOWLABEL: return &ToolbarWindowLabel;
		case SN_TOOLBARCLOCK: return &ToolbarClock;

		case SN_MENUTITLE: return &MenuTitle;
		case SN_MENUFRAME: return &MenuFrame;
		case SN_MENUHILITE: return &MenuHilite;
		case SN_MENUGRIP: return &MenuGrip;

		case SN_MENUBULLET: return &menuBullet;
		case SN_MENUBULLETPOS: return &menuBulletPosition;

		case SN_BORDERWIDTH: return &borderWidth;
		case SN_BORDERCOLOR: return &borderColor;
		case SN_BEVELWIDTH: return &bevelWidth;
		case SN_FRAMEWIDTH: return &frameWidth;
		case SN_HANDLEHEIGHT: return &handleHeight;
		case SN_ROOTCOMMAND: return &rootCommand;
		case SN_MENUALPHA: return &menuAlpha;
		case SN_TOOLBARALPHA: return &toolbarAlpha;
		case SN_METRICSUNIX: return &metricsUnix;
		case SN_BULLETUNIX: return &bulletUnix;

		case SN_WINFOCUS_TITLE: return &windowTitleFocus;
		case SN_WINFOCUS_LABEL: return &windowLabelFocus;
		case SN_WINFOCUS_HANDLE: return &windowHandleFocus;
		case SN_WINFOCUS_GRIP: return &windowGripFocus;
		case SN_WINFOCUS_BUTTON: return &windowButtonFocus;
		case SN_WINFOCUS_BUTTONP: return &windowButtonPressed;
		case SN_WINUNFOCUS_TITLE: return &windowTitleUnfocus;
		case SN_WINUNFOCUS_LABEL: return &windowLabelUnfocus;
		case SN_WINUNFOCUS_HANDLE: return &windowHandleUnfocus;
		case SN_WINUNFOCUS_GRIP: return &windowGripUnfocus;
		case SN_WINUNFOCUS_BUTTON: return &windowButtonUnfocus;

		case SN_WINFOCUS_FRAME_COLOR: return &windowFrameFocusColor;
		case SN_WINUNFOCUS_FRAME_COLOR: return &windowFrameUnfocusColor;

		case SN_ISSTYLE070: return &is_070;
		case SN_SLIT: return &Slit;

			/* BlackboxZero 1.8.2012 */
		case SN_MENUSEPMARGIN: return &MenuSepMargin;
		case SN_MENUSEPCOLOR: return &MenuSepColor;
		case SN_MENUSEPSHADOWCOLOR: return &MenuSepShadowColor;
		case SN_NEWMETRICS: return &nix;

		default: return NULL;
		}
	}


};

//#define STYLESTRUCTSIZE ((SIZEOFPART(StyleStruct, Slit)+3) & ~3)

