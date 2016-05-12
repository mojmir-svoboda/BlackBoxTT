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
#include "bbPager.h"
#include "Drawing.h"
#include "Settings.h"
#include <vector>

// Desktop information
std::vector<RECT> desktopRect;

int col, row, currentCol, currentRow;

//===========================================================================
void DrawBBPager(HWND hwnd)
{
	Settings const & s = getSettings();
	// Create buffer hdc's, bitmaps etc.
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	HDC buf = CreateCompatibleDC(NULL);
	HBITMAP bufbmp = CreateCompatibleBitmap(hdc, s.m_frame.width, s.m_frame.height);
	HGDIOBJ oldbmp = SelectObject(buf, bufbmp);
	RECT r;
	char toolText[256];

	GetClientRect(hwnd, &r);

	// Paint background and border according to the current style...
	MakeStyleGradient(buf, &r, s.m_frame.ownStyle ? s.m_frame.style : &s.m_frame.Style, true);

	HFONT font = CreateStyleFont((StyleItem *)GetSettingPtr(SN_TOOLBARLABEL));
	HGDIOBJ oldfont = SelectObject(buf, font);
	SetBkMode(buf, TRANSPARENT);
	SetTextColor(buf, s.m_desktop.fontColor);
	UINT flags = DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_WORD_ELLIPSIS|DT_NOPREFIX;

	desktopRect.clear();

	// Paint desktops :D
	if (s.m_position.horizontal) 
	{
		// Do loop to draw desktops other than current selected desktop
		int i = 0;

		do 
		{
			col = i / s.m_frame.rows;
			row = i % s.m_frame.rows + 1;

			if (getRuntimeState().m_currentDesktop == i) 
			{
				currentCol = col;
				currentRow = row;
			}
			else 
			{
				r.left = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((col) * (s.m_desktop.width + s.m_frame.bevelWidth));
				r.right = r.left + s.m_desktop.width;
				r.top = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((row - 1) * (s.m_desktop.height + s.m_frame.bevelWidth));
				r.bottom = r.top + s.m_desktop.height;

				//desktopRect[i] = r; // set RECT item for this desktop
				//desktopRect.insert(desktopRect.begin() + i - 1, r);
				desktopRect.push_back(r);

				if (s.m_desktop.ownStyle)
					CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
				else
				{
					r.left  += 1;
					r.top   += 1;
					r.right   -= 1;
					r.bottom  -= 1;
					MakeStyleGradient(buf, &r, &s.m_activeDesktop.Style, false);
				}
				if (s.m_desktop.numbers) 
				{
					char desktopNumber[4];
					sprintf(desktopNumber, "%d", (i + 1));
					DrawText(buf, desktopNumber, -1, &r, flags);
				}
			}
			i++;
		}
		while (i < getRuntimeState().m_desktops);

		// Do this now so bordered desktop is drawn last
		i = getRuntimeState().m_currentDesktop;

		r.left = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((currentCol) * (s.m_desktop.width + s.m_frame.bevelWidth));
		r.right = r.left + s.m_desktop.width;
		r.top = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((currentRow - 1) * (s.m_desktop.height + s.m_frame.bevelWidth));
		r.bottom = r.top + s.m_desktop.height;

		//desktopRect[i] = r; // set RECT item for this desktop

		DrawActiveDesktop(buf, r, i);

		if (s.m_desktop.numbers) 
		{
			char desktopNumber[4];
			sprintf(desktopNumber, "%d", (i + 1));
			DrawText(buf, desktopNumber, -1, &r, flags);
		}
	}
	else if (s.m_position.vertical) 
	{
		// Do loop to draw desktops other than current selected desktop
		int i = 0;

		do 
		{					
			row = i / s.m_frame.columns;
			col = i % s.m_frame.columns + 1;

			if (getRuntimeState().m_currentDesktop == i)
			{
				currentCol = col;
				currentRow = row;
			}
			else 
			{
				r.left = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((col - 1) * (s.m_desktop.width + s.m_frame.bevelWidth));
				r.right = r.left + s.m_desktop.width;
				r.top = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((row) * (s.m_desktop.height + s.m_frame.bevelWidth));
				r.bottom = r.top + s.m_desktop.height;

				//desktopRect[i] = r; // set RECT item for this desktop
				//desktopRect.insert(desktopRect.begin() + i - 1, r);
				desktopRect.push_back(r);

				MakeStyleGradient(buf, &r, s.m_desktop.ownStyle ? s.m_desktop.style : &s.m_desktop.Style, false);
				if (s.m_desktop.ownStyle)
					CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
				else
				{
					r.left  += 1;
					r.top   += 1;
					r.right   -= 1;
					r.bottom  -= 1;
					MakeStyleGradient(buf, &r, &s.m_activeDesktop.Style, false);
				}
				if (s.m_desktop.numbers) 
				{
					char desktopNumber[4];
					sprintf(desktopNumber, "%d", (i + 1));
					DrawText(buf, desktopNumber, -1, &r, flags);
				}
			}
			i++;
		}
		while (i < getRuntimeState().m_desktops);

		// Do this now so bordered desktop is drawn last
		i = getRuntimeState().m_currentDesktop;

		r.left = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((currentCol - 1) * (s.m_desktop.width + s.m_frame.bevelWidth));
		r.right = r.left + s.m_desktop.width;
		r.top = s.m_frame.borderWidth + s.m_frame.bevelWidth + ((currentRow) * (s.m_desktop.height + s.m_frame.bevelWidth));
		r.bottom = r.top + s.m_desktop.height;

		//desktopRect[i] = r; // set RECT item for this desktop

		DrawActiveDesktop(buf, r, i);

		if (s.m_desktop.numbers) 
		{
			char desktopNumber[4];
			sprintf(desktopNumber, "%d", (i + 1));
			SetTextColor(buf, s.m_activeDesktop.borderColor);
			DrawText(buf, desktopNumber, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
			//DrawText(buf, desktopNumber, strlen(desktopNumber), &r, DT_CALCRECT|DT_NOPREFIX);
			//SetTextColor(buf, s.m_desktop.fontColor);
		}
	}

	//DeleteObject(font);
	DeleteObject(SelectObject(buf, oldfont));

	// Draw windows on workspaces if wanted
	if (s.m_desktop.windows)
	{
		getRuntimeState().m_winCount = 0; // Reset number of windows to 0 on each paint to be counted by...
		getRuntimeState().m_winList.clear();

		// ... this function which passes HWNDs to CheckTaskEnumProc callback procedure
		if (!getRuntimeState().m_is_xoblite && getRuntimeState().m_usingAltMethod)
			EnumWindows(CheckTaskEnumProc_AltMethod, 0);
		else
			EnumWindows(CheckTaskEnumProc, 0); 

		//struct tasklist *tlist;
		/*tl = GetTaskListPtr();
		while (tl)
		{
			AddBBWindow(tl);
			tl = tl->next;
		}*/

		// Only paint windows if there are any!
		if (getRuntimeState().m_winCount > 0)
		{
			// Start at end of list (bottom of zorder)
			for (int i = (getRuntimeState().m_winCount - 1); i > -1; i--)
			{
				RECT win = getRuntimeState().m_winList[i].r;
				RECT desk = desktopRect[getRuntimeState().m_winList[i].desk];

				if (win.right - win.left <= 1 && win.bottom - win.top <= 1)
					continue;
				
				// This is done so that windows only show within the applicable desktop RECT
				if (win.top < desk.top) 
					win.top = desk.top; // + 1;

				if (win.right > desk.right) 
					win.right = desk.right; // - 1;

				if (win.bottom > desk.bottom) 
					win.bottom = desk.bottom; // - 1;

				if (win.left < desk.left) 
					win.left = desk.left; // + 1;

				if (getRuntimeState().m_winList[i].sticky)
				{
					RECT sWin;
					RECT sDesk;
					win.bottom = win.bottom - desk.top;
					win.top = win.top - desk.top;
					win.left = win.left - desk.left;
					win.right = win.right - desk.left;

					for (int j = 0; j < getRuntimeState().m_desktops; j++)
					{
						sDesk = desktopRect[j];
						sWin.bottom = sDesk.top + win.bottom;
						sWin.top = sDesk.top + win.top;
						sWin.left = sDesk.left + win.left;
						sWin.right = sDesk.left + win.right;

						if (getRuntimeState().m_winList[i].active) // draw active window style
						{
							DrawActiveWindow(buf, sWin);
							RemoveFlash(getRuntimeState().m_winList[i].window, true);
						}
						else if (IsFlashOn(getRuntimeState().m_winList[i].window))
						{
							DrawActiveWindow(buf, sWin);
						}
						else // draw inactive window style
						{
							DrawInactiveWindow(buf, sWin);
							RemoveFlash(getRuntimeState().m_winList[i].window, true);
						}

						// Create a tooltip...
						if (s.m_desktop.tooltips)
						{
							GetWindowText(getRuntimeState().m_winList[i].window, toolText, 255);
							SetToolTip(&sWin, toolText);
						}
					}
				}
				else
				{
					if (getRuntimeState().m_winList[i].active) // draw active window style
					{
						DrawActiveWindow(buf, win);
						RemoveFlash(getRuntimeState().m_winList[i].window, true);
					}
					else if (IsFlashOn(getRuntimeState().m_winList[i].window))
					{
						DrawActiveWindow(buf, win);
					}
					else // draw inactive window style
					{
						DrawInactiveWindow(buf, win);
						RemoveFlash(getRuntimeState().m_winList[i].window, true);
					}

					// Create a tooltip...
					if (s.m_desktop.tooltips)
					{
						GetWindowText(getRuntimeState().m_winList[i].window, toolText, 255);
						SetToolTip(&win, toolText);
					}
				}
			}
		}

		if (getRuntimeState().m_winMoving)
		{
			RECT win = getRuntimeState().m_moveWin.r;
			RECT client;

			GetClientRect(getRuntimeState().m_hwndBBPager, &client);
		
			// This is done so that the window only shows within the pager
			if (win.top < client.top) 
				win.top = client.top; // + 1;

			if (win.right > client.right) 
				win.right = client.right; // - 1;

			if (win.bottom > client.bottom) 
				win.bottom = client.bottom; // - 1;

			if (win.left < client.left) 
				win.left = client.left; // + 1;

			if (getRuntimeState().m_moveWin.active) // draw active window style
				DrawActiveWindow(buf, win);
			else // draw inactive window style
				DrawInactiveWindow(buf, win);
		}

	}

	ClearToolTips();

	// Finally, copy from the paint buffer to the window...
	BitBlt(hdc, 0, 0, s.m_frame.width, s.m_frame.height, buf, 0, 0, SRCCOPY);

    //restore the first previous whatever to the dc,
    //get in exchange back our bitmap, and delete it.
    DeleteObject(SelectObject(buf, oldbmp));

    //delete the memory - 'device context'
    DeleteDC(buf);

    //done
    EndPaint(hwnd, &ps);
}

//===========================================================================
void DrawActiveWindow(HDC buf, RECT r)
{
	COLORREF bColor;
	Settings const & s = getSettings();

	// Checks for windows just showing on the edges of the screen
	if (r.bottom - r.top < 2)
	{
		if (!_stricmp(s.m_focusedWindow.styleType, "border"))
			bColor = s.m_focusedWindow.borderColor;
		else
			bColor = s.m_window.borderColor;

		HPEN borderPen = CreatePen(PS_SOLID, 1, bColor);
		HPEN oldPen = (HPEN) SelectObject(buf, borderPen);

		MoveToEx(buf, r.left, r.top, NULL);
		LineTo(buf, r.right, r.top);

		SelectObject(buf,oldPen);
		DeleteObject(borderPen);

		//MessageBox(0, "Warning sir!\n\nCan't draw this window as a RECT dude!", "DrawActiveWindow", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);

		return;
	}

	if (r.right - r.left < 2)
	{
		if (!_stricmp(s.m_focusedWindow.styleType, "border"))
			bColor = s.m_focusedWindow.borderColor;
		else
			bColor = s.m_window.borderColor;

		HPEN borderPen = CreatePen(PS_SOLID, 1, bColor);
		HPEN oldPen = (HPEN) SelectObject(buf, borderPen);

		MoveToEx(buf, r.left, r.top, NULL);
		LineTo(buf, r.left, r.bottom);

		SelectObject(buf,oldPen);
		DeleteObject(borderPen);

		return;
	}

	if (!_stricmp(s.m_focusedWindow.styleType, "texture"))
	{
		MakeStyleGradient(buf, &r, s.m_focusedWindow.ownStyle ? s.m_focusedWindow.style : &s.m_focusedWindow.Style, false);
		CreateBorder(buf, &r, s.m_focusedWindow.borderColor, 1);
	}
	else if (!_stricmp(s.m_focusedWindow.styleType, "border"))
	{
		MakeStyleGradient(buf, &r, s.m_window.ownStyle ? s.m_window.style : &s.m_window.Style, false);
		CreateBorder(buf, &r, s.m_focusedWindow.borderColor, 1);
	}
	else
		MakeStyleGradient(buf, &r, s.m_window.style, true);
}

void DrawInactiveWindow(HDC buf, RECT r)
{
	Settings const & s = getSettings();
	if (r.bottom - r.top < 2)
	{
		HPEN borderPen = CreatePen(PS_SOLID, 1, getSettings().m_window.borderColor);
		HPEN oldPen = (HPEN) SelectObject(buf, borderPen);

		MoveToEx(buf, r.left, r.top, NULL);
		LineTo(buf, r.right, r.top);

		SelectObject(buf,oldPen);
		DeleteObject(borderPen);

		return;
	}

	if (r.right - r.left < 2)
	{
		HPEN borderPen = CreatePen(PS_SOLID, 1, getSettings().m_window.borderColor);
		HPEN oldPen = (HPEN) SelectObject(buf, borderPen);

		MoveToEx(buf, r.left, r.top, NULL);
		LineTo(buf, r.left, r.bottom);

		SelectObject(buf,oldPen);
		DeleteObject(borderPen);

		return;
	}

	MakeStyleGradient(buf, &r, s.m_window.ownStyle ? s.m_window.style : &s.m_window.Style, false);
		CreateBorder(buf, &r, s.m_window.borderColor, 1);
}

//===========================================================================
void DrawActiveDesktop(HDC buf, RECT r, int i)
{
	Settings const & s = getSettings();
	if (!_stricmp(s.m_activeDesktop.styleType, "border3")) 
	{
		r.right = r.right + 2;
		r.bottom = r.bottom + 2;

		if (s.m_activeDesktop.Style.parentRelative)
			CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
		else
		{
			MakeStyleGradient(buf, &r, s.m_activeDesktop.ownStyle ? s.m_activeDesktop.style : &s.m_activeDesktop.Style, false);
			CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
		}
	}
	else if (!_stricmp(s.m_activeDesktop.styleType, "border2")) 
	{
		r.left = r.left - 1;
		r.top = r.top - 1;
		r.right = r.right + 1;
		r.bottom = r.bottom + 1;

		if (!s.m_activeDesktop.ownStyle && s.m_activeDesktop.Style.parentRelative)
			CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
		else
		{
			MakeStyleGradient(buf, &r, s.m_activeDesktop.ownStyle ? s.m_activeDesktop.style : &s.m_activeDesktop.Style, false);
			CreateBorder(buf, &r, s.m_desktop.Style.borderColor, 1);
		}
	}
	else if (!_stricmp(s.m_activeDesktop.styleType, "border")) 
	{
		if (!s.m_activeDesktop.ownStyle && s.m_activeDesktop.Style.parentRelative)
			CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
		else
		{
			MakeStyleGradient(buf, &r, s.m_activeDesktop.ownStyle ? s.m_activeDesktop.style : &s.m_activeDesktop.Style, false);
			CreateBorder(buf, &r, s.m_desktop.Style.borderColor, 1);
		}
	}
	else if (!_stricmp(s.m_activeDesktop.styleType, "texture")) 
	{
		if (!s.m_activeDesktop.ownStyle && !s.m_activeDesktop.Style.parentRelative)
			MakeStyleGradient(buf, &r, s.m_activeDesktop.ownStyle ? s.m_activeDesktop.style : &s.m_activeDesktop.Style, false);
		CreateBorder(buf, &r, s.m_activeDesktop.borderColor, 1);
	}
	else if (!s.m_desktop.Style.parentRelative) // "none"
		MakeStyleGradient(buf, &r, s.m_desktop.ownStyle ? s.m_desktop.style : &s.m_desktop.Style, false);	

	//desktopRect[i] = r;
	desktopRect.insert(desktopRect.begin() + i, r);
}


