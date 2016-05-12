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
#include "Settings.h"
#include "bbPager.h"
#include <blackbox/Workspaces.h>

//===========================================================================
char rcpath[MAX_PATH];
char bspath[MAX_PATH];
char * getbspath () { return bspath; }
char stylepath[MAX_PATH];

Settings g_settings;
Settings & getSettings () { return g_settings; } // "singleton"

//===========================================================================
inline bool is_slash(char c) { return c == '\\' || c == '/'; }

int File_Exists(const char* szFileName)
{
    DWORD a = GetFileAttributes(szFileName);
    return (DWORD)-1 != a && 0 == (a & FILE_ATTRIBUTE_DIRECTORY);
}

int locate_file(HINSTANCE hInstance, char *path, const char *fname, const char *ext)
{
	GetModuleFileName(hInstance, path, MAX_PATH);
	char *file_name_start = strrchr(path, '\\');
	if (file_name_start) ++file_name_start;
	else file_name_start = strchr(path, 0);
	sprintf(file_name_start, "%s.%s", fname, ext);
	return File_Exists(path);
}

/*const char *file_basename(const char *path)
{
    int nLen = strlen(path);
    while (nLen && !is_slash(path[nLen-1])) nLen--;
    return path + nLen;
}

int imin(int a, int b)
{
    return a<b?a:b;
}*/

void Settings::ReadRCSettings ()
{
	char temp[16];

	// If a config file was not found we use the defaults...
		
	// Position of BBPager window
	int n, n1, n2;
	strcpy(temp, ReadString(rcpath, "bbPager.position:", ""));
	n = sscanf(temp, "+%d-%d", &n1, &n2);

	if (n == 2)
	{
		m_position.unix = true;
		m_xpos = m_position.x = m_position.ox = n1;
		m_ypos = m_position.y = m_position.oy = n2;
	}
	else 
	{
		m_position.unix = false;
		m_xpos = m_position.x = m_position.ox = ReadInt(rcpath, "bbPager.position.x:", 895);
		m_ypos = m_position.y = m_position.oy = ReadInt(rcpath, "bbPager.position.y:", 10);
	}

	m_position.raised = ReadBool(rcpath, "bbPager.raised:", true);
	m_position.snapWindow = ReadInt(rcpath, "bbPager.snapWindow:", 20);

	// BBPager metrics
	m_desktop.width = ReadInt(rcpath, "bbPager.desktop.width:", 40);
	m_desktop.height  = ReadInt(rcpath, "bbPager.desktop.height:", 30);

	UpdateMonitorInfo();

	m_desktop.sizeRatio = ReadInt(rcpath, "bbPager.sizeRatio:", 20);
	m_desktop.width = m_vScreenWidth / m_desktop.sizeRatio;
	m_desktop.height = m_vScreenHeight / m_desktop.sizeRatio;

	// get mouse button for desktop changing, etc., 1 = LMB, 2 = Middle, 3 = RMB
	m_desktopChangeButton = ReadInt(rcpath, "bbPager.desktopChangeButton:", 2);
	m_focusButton = ReadInt(rcpath, "bbPager.windowFocusButton:", 1);
	m_moveButton = ReadInt(rcpath, "bbPager.windowMoveButton:", 3);
 
	// default BB editor
	GetBlackboxEditor(m_editor);
	if (strlen(m_editor) < 2)
		strcpy(m_editor, "notepad.exe");

	//get vertical or horizontal alignment setting
	strcpy(temp, ReadString(rcpath, "bbPager.alignment:", "horizontal"));
	if (!_stricmp(temp, "vertical")) 
	{
		m_position.horizontal = false;
		m_position.vertical = true;
	}
	else 
	{
		m_position.horizontal = true;
		m_position.vertical = false;
	}

	DesktopInfo DI;
	getWorkspaces().GetDesktopInfo(DI);

	// row and column number
	m_frame.columns = DI.nScreensX;
	m_frame.rows = DI.nScreensY;
	//m_frame.columns = ReadInt(rcpath, "bbPager.columns:", 1);
	//m_frame.rows = ReadInt(rcpath, "bbPager.rows:", 1);

	if (m_frame.rows < 1) m_frame.rows = 1;
	if (m_frame.columns < 1) m_frame.columns = 1;
	if (m_frame.rows > 1 && m_frame.columns > 1)
		m_position.grid = true;

	//numbers on desktop enable
	m_desktop.numbers = ReadBool(rcpath, "bbPager.desktopNumbers:", false);

	//windows on desktop enable
	m_desktop.windows = ReadBool(rcpath, "bbPager.desktopWindows:", true);
	m_desktop.tooltips = ReadBool(rcpath, "bbPager.windowToolTips:", false);

	// Autohide enable
	m_position.autohide = ReadBool(rcpath, "bbPager.autoHide:", false);

	m_topMargin = ReadInt(extensionsrcPath(), "blackbox.desktop.marginTop:", 0);
	m_leftMargin = ReadInt(extensionsrcPath(), "blackbox.desktop.marginLeft:", 0);
	m_usingAltMethod = ReadBool(extensionsrcPath(), "blackbox.workspaces.altMethod:", false);

	// Transparency
	m_transparency = ReadBool(rcpath, "bbPager.transparency:", false);
	m_transparencyAlpha = ReadInt(rcpath, "bbPager.transparency.alpha:", 192);

	m_useSlit = ReadBool(rcpath, "bbPager.useSlit:", false);

	if (!locate_file(getRuntimeState().m_hInstance, (char*)rcpath, "bbPager", "rc"))
		WriteRCSettings();
}

//===========================================================================

void Settings::WriteRCSettings ()
{
	static char szTemp[128];
	static char temp[32];
	DWORD retLength = 0;

	// Write plugin settings to config file, using path found in ReadRCSettings()...
	HANDLE file = CreateFile(rcpath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		// BBPager position
		if (m_position.unix) 
		{
			sprintf(szTemp, "bbPager.position: +%d-%d\r\n", m_position.ox, m_position.oy);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		}
		else 
		{
			sprintf(szTemp, "bbPager.position.x: %d\r\n", m_position.ox);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
			sprintf(szTemp, "bbPager.position.y: %d\r\n", m_position.oy);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		}

		// desktop size
		sprintf(szTemp, "bbPager.desktop.width: %d\r\n", m_desktop.width);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		sprintf(szTemp, "bbPager.desktop.height: %d\r\n", m_desktop.height);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		sprintf(szTemp, "bbPager.desktop.sizeRatio: %d\r\n", m_desktop.sizeRatio);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// alignment
		(m_position.vertical) ? strcpy(temp, "vertical") : strcpy(temp, "horizontal");
		sprintf(szTemp, "bbPager.alignment: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Write column/row values
		sprintf(szTemp, "bbPager.columns: %d\r\n", m_frame.columns);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbPager.rows: %d\r\n", m_frame.rows);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// desktop change mouse button, etc.
		sprintf(szTemp, "bbPager.desktopChangeButton: %d\r\n", m_desktopChangeButton);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbPager.windowMoveButton: %d\r\n", m_moveButton);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbPager.windowFocusButton: %d\r\n", m_focusButton);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Snap window to edge of screen
		sprintf(szTemp, "bbPager.snapWindow: %d\r\n", m_position.snapWindow);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Always on top
		(m_position.raised) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.raised: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Autohide
		(m_position.autohide) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.autoHide: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Transparency
		if (getRuntimeState().m_usingWin2kXP)
		{
			(m_transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
			sprintf(szTemp, "bbPager.transparency: %s\r\n", temp);
 			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbPager.transparency.alpha: %d\r\n", m_transparencyAlpha);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		}

		// Numbers on Desktops
		(m_desktop.numbers) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.desktopNumbers: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Windows on Desktops
		(m_desktop.windows) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.desktopWindows: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(m_usingAltMethod) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.desktopAltMethod: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(m_desktop.tooltips) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.windowToolTips: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		// Are we in the slit?
		(m_useSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbPager.useSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
 	}
	CloseHandle(file);
}

//===========================================================================

void Settings::UpdateMonitorInfo ()
{
	// multimonitor
	m_vScreenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_vScreenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

	m_vScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	m_vScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	m_vScreenRight = m_vScreenLeft + m_vScreenWidth;
	m_vScreenBottom = m_vScreenTop + m_vScreenHeight;

	m_ratioX = m_vScreenWidth / m_desktop.width;
	m_ratioY = m_vScreenHeight / m_desktop.height;

	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	int yScreen = GetSystemMetrics(SM_CYSCREEN);

	if (m_vScreenWidth > xScreen || m_vScreenHeight > yScreen)
	{	// multimon
		// current monitor
		HMONITOR hMon = MonitorFromWindow(getRuntimeState().m_hwndBBPager, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mInfo;
		mInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMon, &mInfo);

		m_screenLeft = mInfo.rcMonitor.left;
		m_screenRight = mInfo.rcMonitor.right;
		m_screenTop = mInfo.rcMonitor.top;
		m_screenBottom = mInfo.rcMonitor.bottom;
		m_screenWidth = m_screenRight - m_screenLeft;
		m_screenHeight = m_screenBottom - m_screenTop;
	}
	else	// single mon (or treat as such)
	{
		m_vScreenTop = m_vScreenLeft = 0;

		m_screenLeft = 0;
		m_screenRight = m_screenWidth;
		m_screenTop = 0;
		m_screenBottom = m_screenHeight;
		m_screenWidth = m_vScreenWidth = xScreen;
		m_screenHeight = m_vScreenHeight = yScreen;

		m_ratioX = m_screenWidth / m_desktop.width;
		m_ratioY = m_screenHeight / m_desktop.height;
	}
}

//===========================================================================

void Settings::GetStyleSettings()
{
	char tempstring[256];
	char colorAsString[32] = "#000000";

	// Get the path to the current style file from Blackbox...
	strcpy(stylepath, stylePath());
	// Get the path to the plugin.bb file
	locate_file(getRuntimeState().m_hInstance, (char*)bspath, "bbPager", "bb");

	bool nix = false;
	if (!getRuntimeState().m_is_xoblite)
		nix = 0 == (int)GetSettingPtr(SN_NEWMETRICS);
	else 
	{
		strcpy(tempstring, ReadString(stylepath, "toolbar.appearance:", "no"));
		if (strlen(tempstring) != 2) nix = true;
	}

	//===========================================================
	// bbpager.frame: -> this is for the BBPager frame/background
	m_frame.ownStyle = false;
	m_frame.Style = *(StyleItem *)GetSettingPtr(SN_TOOLBAR);

	strcpy(tempstring, ReadString(bspath, "bbPager.frame:", ReadString(stylepath, "bbPager.frame:", "no")));
	if (strlen(tempstring) != 2)
 	{
		m_frame.ownStyle = true;
 		if (m_frame.style)
			delete m_frame.style;
 		m_frame.style = new StyleItem;
		ParseItem(tempstring, m_frame.style);
		m_frame.color = ReadColor(bspath, "bbPager.frame.color:", ReadString(stylepath, "bbPager.frame.color:", colorAsString));
		m_frame.colorTo = ReadColor(bspath, "bbPager.frame.colorTo:", ReadString(stylepath, "bbPager.frame.colorTo:", colorAsString));
 	}

	//===========================================================
	// bbpager.frame.borderColor: -> this is the colour for the border around BBPager
	m_frame.borderColor = ReadColor(bspath, "bbPager.frame.borderColor:", ReadString(stylepath, "bbPager.frame.borderColor:", ReadString(stylepath, "toolbarLabel.TextColor:", ReadString(stylepath, "borderColor:", colorAsString))));

	//===========================================================
	// bbpager.desktop: -> this is for the normal desktops
	m_desktop.ownStyle = false;

	m_desktop.Style = *(StyleItem *)GetSettingPtr(SN_TOOLBARBUTTON);
	if (m_desktop.Style.parentRelative)
		m_desktop.Style = *(StyleItem *)GetSettingPtr(SN_TOOLBARLABEL);
 
	strcpy(tempstring, ReadString(bspath, "bbPager.desktop:", ReadString(stylepath, "bbPager.desktop:", "no")));
	if (strlen(tempstring) != 2)
 	{
		m_desktop.ownStyle = true;
		if (m_desktop.style) delete m_desktop.style;
		m_desktop.style = new StyleItem;
		ParseItem(tempstring, m_desktop.style);
		m_desktop.color = ReadColor(bspath, "bbPager.desktop.color:", ReadString(stylepath, "bbPager.desktop.color:", colorAsString));
		m_desktop.colorTo = ReadColor(bspath, "bbPager.desktop.colorTo:", ReadString(stylepath, "bbPager.desktop.colorTo:", colorAsString));
 	}

	m_desktop.Style.bevelstyle = BEVEL_RAISED; 
	m_desktop.Style.bevelposition = BEVEL2; 

	//===========================================================
	// bbpager.desktop.focusStyle: -> specifies how to draw active desktop - none|border|border2|border3|texture

	strcpy(m_activeDesktop.styleType, ReadString(bspath, "bbPager.desktop.focusStyle:", ReadString(stylepath, "bbPager.desktop.focusStyle:", "border")));

	//===========================================================
	// bbpager.desktop.focus: -> style definition used for current workspace

	m_activeDesktop.ownStyle = false;

	m_activeDesktop.Style = *(StyleItem *)GetSettingPtr(SN_TOOLBARBUTTONP);
	if (m_activeDesktop.Style.parentRelative)
		m_activeDesktop.Style = *(StyleItem *)GetSettingPtr(SN_TOOLBARWINDOWLABEL);

	strcpy(tempstring, ReadString(bspath, "bbPager.desktop.focus:", ReadString(stylepath, "bbPager.desktop.focus:", "no")));
	if (strlen(tempstring) != 2)
 	{
		m_activeDesktop.ownStyle = true;
		if (m_activeDesktop.style)
			delete m_activeDesktop.style;
		m_activeDesktop.style = new StyleItem;
		ParseItem(tempstring, m_activeDesktop.style);
		m_activeDesktop.color = ReadColor(bspath, "bbPager.desktop.focus.color:", ReadString(stylepath, "bbPager.desktop.focus.color:", colorAsString));
		m_activeDesktop.colorTo = ReadColor(bspath, "bbPager.desktop.focus.colorTo:", ReadString(stylepath, "bbPager.desktop.focus.colorTo:", colorAsString));
 	}

	m_activeDesktop.Style.bevelstyle = BEVEL_SUNKEN; 

	//===========================================================
	// bbpager.active.desktop.borderColor:

	m_activeDesktop.borderColor = ReadColor(bspath, "bbPager.active.desktop.borderColor:", ReadString(stylepath, "bbPager.active.desktop.borderColor:", ReadString(stylepath, "borderColor:", colorAsString)));
	if (m_activeDesktop.borderColor == 0x000000)
		m_activeDesktop.borderColor = m_activeDesktop.Style.TextColor;

	//===========================================================
	// frame.bevelWidth: spacing between desktops and edges of BBPager
	m_frame.bevelWidth = m_frame.ownStyle ? ReadInt(bspath, "bbPager.bevelwidth:", *(int *)GetSettingPtr(SN_BEVELWIDTH)) : (nix ? m_frame.Style.marginWidth : *(int *)GetSettingPtr(SN_BEVELWIDTH));

	//===========================================================
	// frame.borderWidth: width of border around BBPager

	m_frame.borderWidth = m_frame.ownStyle ? ReadInt(bspath, "bbPager.frame.borderWidth:", *(int *)GetSettingPtr(SN_BORDERWIDTH)) : (nix ? m_frame.Style.borderWidth : *(int *)GetSettingPtr(SN_BORDERWIDTH));

	//===========================================================
	// frame.hideWidth: amount of pager seen in the hidden state

	m_frame.hideWidth = imin(m_frame.bevelWidth + m_frame.borderWidth, 3);

	//===========================================================
	// bbpager.window: -> this is for the normal windows

	m_window.Style = *(StyleItem *)GetSettingPtr(SN_MENUFRAME);

	m_window.ownStyle = false;
	if (m_window.style)
		delete m_window.style;
	m_window.style = new StyleItem;

	strcpy(tempstring, ReadString(bspath, "bbPager.window:", "no"));	
	if (strlen(tempstring) == 2) // plugin.window NOT in BB
	{
		strcpy(tempstring, ReadString(stylepath, "bbPager.window:", "no"));	
		if (strlen(tempstring) != 2) // plugin.window IS in STYLE
			m_window.ownStyle = true;
	}
	else // plugin.window IS in BB
		m_window.ownStyle = true;

	bool labelPR = false;
	strcpy(tempstring, ReadString(bspath, "window.label.unfocus:", ReadString(stylepath, "window.label.unfocus.appearance:", "parentrelative")));
	if (strlen(tempstring) == 14)
		labelPR = true;

	if (m_window.ownStyle)
	{
		if (labelPR)
		{
			strcpy(tempstring, ReadString(bspath, "bbPager.window:", ReadString(stylepath, "bbPager.window:", ReadString(bspath, "window.title.unfocus:", ReadString(bspath, "window.title.unfocus.appearance:", "Sunken Bevel2 Gradient Vertical")))));
			ParseItem(tempstring, m_window.style);

			m_window.color = ReadColor(bspath, "bbPager.window.color:", ReadString(stylepath, "bbPager.window.color:", ReadString(stylepath, "window.title.unfocus.color:", ReadString(stylepath, "window.title.unfocus.color1:", ReadString(stylepath, "window.title.unfocus.backgroundColor:", colorAsString)))));
			if (m_window.style->type == B_SOLID)
				m_window.colorTo = m_window.color;
			else
				m_window.colorTo = ReadColor(bspath, "bbPager.window.colorTo:", ReadString(stylepath, "bbPager.window.colorTo:", ReadString(stylepath, "window.title.unfocus.colorTo:", ReadString(stylepath, "window.title.unfocus.color2:", colorAsString))));
		}
		else
		{
			m_window.color = ReadColor(bspath, "bbPager.window.color:", ReadString(stylepath, "bbPager.window.color:", ReadString(stylepath, "window.label.unfocus.color:", ReadString(stylepath, "window.label.unfocus.color1:", ReadString(stylepath, "window.label.unfocus.backgroundColor:", colorAsString)))));
			if (m_window.style->type == B_SOLID)
				m_window.colorTo = m_window.color;
			else
				m_window.colorTo = ReadColor(bspath, "bbPager.window.colorTo:", ReadString(stylepath, "bbPager.window.colorTo:", ReadString(stylepath, "window.label.unfocus.colorTo:", ReadString(stylepath, "window.label.unfocus.color2:", colorAsString))));
		}
	}

	m_window.Style.bevelstyle = BEVEL_SUNKEN; 

	//===========================================================
	// bbpager.inactive.window.borderColor:

	m_window.borderColor = ReadColor(bspath, "bbPager.inactive.window.borderColor:", ReadString(stylepath, "bbPager.inactive.window.borderColor:", ReadString(stylepath, "window.label.unfocus.TextColor:", colorAsString)));
	if (m_window.borderColor == 0x000000)
		m_window.borderColor = m_activeDesktop.Style.Color;

	//===========================================================
	// bbpager.window.focusStyle: -> specifies how to draw active window - none|border|border2|border3|texture

	strcpy(m_focusedWindow.styleType, ReadString(bspath, "bbPager.window.focusStyle:", ReadString(stylepath, "bbPager.window.focusStyle:", "texture")));

	//===========================================================
	// bbpager.window.focus: -> style definition used for active window

	m_focusedWindow.Style = *(StyleItem *)GetSettingPtr(SN_MENUHILITE);

	m_focusedWindow.ownStyle = false;
	if (m_focusedWindow.style) delete m_focusedWindow.style;
	m_focusedWindow.style = new StyleItem;

	strcpy(tempstring, ReadString(bspath, "bbPager.window.focus:", "no"));	
	if (strlen(tempstring) == 2) // plugin.active.desktop NOT in BB
	{
		strcpy(tempstring, ReadString(stylepath, "bbPager.window.focus:", "no"));	
		if (strlen(tempstring) != 2) // plugin.window.focus IS in STYLE
			m_focusedWindow.ownStyle = true;
	}
	else // plugin.active.desktop IS in BB
		m_focusedWindow.ownStyle = true;

	if (m_focusedWindow.ownStyle)
	{
		if (labelPR)
		{
			strcpy(tempstring, ReadString(bspath, "bbPager.focus.window:", ReadString(stylepath, "bbPager.focus.window:", ReadString(bspath, "window.title.focus:", ReadString(bspath, "window.title.focus.appearance:", "Raised Bevel2 Gradient Vertical")))));
			ParseItem(tempstring, m_focusedWindow.style);

			m_focusedWindow.color = ReadColor(bspath, "bbPager.window.focus.color:", ReadString(stylepath, "bbPager.window.focus.color:", ReadString(stylepath, "window.title.focus.color:", ReadString(stylepath, "window.title.focus.color1:", ReadString(stylepath, "window.title.focus.backgroundColor:", colorAsString)))));
			if (m_focusedWindow.style->type == B_SOLID)
				m_focusedWindow.colorTo = m_focusedWindow.color;
			else
				m_focusedWindow.colorTo = ReadColor(bspath, "bbPager.window.focus.colorTo:", ReadString(stylepath, "bbPager.window.focus.colorTo:", ReadString(stylepath, "window.title.focus.colorTo:", ReadString(stylepath, "window.title.focus.color2:", colorAsString))));
		}
		else
		{
			strcpy(tempstring, ReadString(bspath, "bbPager.focus.window:", ReadString(stylepath, "bbPager.focus.window:", ReadString(bspath, "window.label.focus:", ReadString(bspath, "window.label.focus.appearance:", "Raised Bevel2 Gradient Vertical")))));
			ParseItem(tempstring, m_focusedWindow.style);

			m_focusedWindow.color = ReadColor(bspath, "bbPager.window.focus.color:", ReadString(stylepath, "bbPager.window.focus.color:", ReadString(stylepath, "window.label.focus.color:", ReadString(stylepath, "window.label.focus.color1:", ReadString(stylepath, "window.label.focus.backgroundColor:", colorAsString)))));
			if (m_focusedWindow.style->type == B_SOLID)
				m_focusedWindow.colorTo = m_focusedWindow.color;
			else
				m_focusedWindow.colorTo = ReadColor(bspath, "bbPager.window.focus.colorTo:", ReadString(stylepath, "bbPager.window.focus.colorTo:", ReadString(stylepath, "window.label.focus.colorTo:", ReadString(stylepath, "windowlabel.focus.color2:", colorAsString))));
		}
	}

	m_focusedWindow.Style.bevelstyle = BEVEL_RAISED;
	m_focusedWindow.Style.bevelposition = BEVEL2;

	//===========================================================
	// bbpager.active.window.borderColor:
	m_focusedWindow.borderColor = ReadColor(bspath, "bbPager.active.window.borderColor:", ReadString(stylepath, "bbPager.active.window.borderColor:", ReadString(stylepath, "window.label.focus.TextColor:", colorAsString)));
	if (m_focusedWindow.borderColor == 0x000000)
		m_focusedWindow.borderColor = m_frame.Style.Color;
}

