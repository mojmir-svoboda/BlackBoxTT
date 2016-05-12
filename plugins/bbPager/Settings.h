#pragma once
#include "bbPager.h"

struct Settings
{
	int m_xpos, m_ypos;
	DESKTOP m_desktop;
	FRAME m_frame;
	POSITION m_position;
	FOCUSEDWINDOW m_focusedWindow;
	ACTIVEDESKTOP m_activeDesktop;
	WINDOW m_window;
	int m_desktopChangeButton;
	int m_focusButton;
	int m_moveButton;

	// Window information
	int m_screenWidth, m_screenHeight, m_screenLeft, m_screenTop, m_screenRight, m_screenBottom;
	int m_vScreenWidth, m_vScreenHeight, m_vScreenLeft, m_vScreenTop, m_vScreenRight, m_vScreenBottom;
	double m_ratioX, m_ratioY;
	int m_leftMargin, m_topMargin;
	bool m_drawBorder;

	// Transparency
	bool m_transparency;
	int m_transparencyAlpha;

	bool m_useSlit;
	bool m_usingAltMethod;
	bool m_usingWin2kXP;

	char m_editor[MAX_PATH];

	Settings () { memset(this, 0, sizeof(*this)); }
	void ReadRCSettings ();
	void WriteRCSettings ();
	void UpdateMonitorInfo ();
	void GetStyleSettings (); // split with m_focusedWindow into another class? 
};

char * getbspath ();
Settings & getSettings ();
