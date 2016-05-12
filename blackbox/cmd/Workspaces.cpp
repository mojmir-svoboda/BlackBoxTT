/* ==========================================================================

  This file is part of the bbLean source code
  Copyright © 2001-2003 The Blackbox for Windows Development Team
  Copyright © 2004-2009 grischka

  http://bb4win.sourceforge.net/bblean
  http://developer.berlios.de/projects/bblean

  bbLean is free software, released under the GNU General Public License
  (GPL version 2). For details see:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  ========================================================================== */

#include "BB.h"
#include "Settings.h"
#include "Workspaces.h"
#include "Desk.h"
#include "BBVWM.h"
#include "MessageManager.h"
#include <lib2/bblib2.h>

Workspaces g_Workspaces;
Workspaces & getWorkspaces () { return g_Workspaces; }

Workspaces::Workspaces ()
    : nScreens(1)
    , nScreensX(1), nScreensY(1)
    , currentScreen(0)
    , lastScreen(0)
    , VScreenX(0), VScreenY(0), VScreenWidth(0), VScreenHeight(0)
    , deskNames(0)
    , stickyNamesList(0)
    , onBGNamesList(0)
    , taskList(0)
    , pTopTask(0)
    , activeTaskWindow(0)
    , toggled_windows(0)
    , sticky_list(0)
    , onbg_list(0)
{ }

Workspaces::~Workspaces ()
{ }

//====================
// local functions

/*void switchToDesktop(int desk);
void setDesktop(HWND hwnd, int desk, bool switchto);
int is_valid_task(HWND hwnd);
HWND get_top_window(int scrn);

void send_desk_refresh(void);
void send_task_refresh(void);*/

//===========================================================================
void Workspaces::Init (int nostartup)
{
    currentScreen   = 0;
    lastScreen      = 0;
    deskNames       = NULL;
    stickyNamesList = NULL;
    onBGNamesList   = NULL;

    SetNames();
    WS_LoadStickyNamesList();
    WS_LoadOnBGNamesList();
    GetScreenMetrics();
    vwm_init();
    if (!nostartup)
        init_tasks();
}

void Workspaces::Exit ()
{
    exit_tasks();
    vwm_exit();
    freeall(&deskNames);
    freeall(&onBGNamesList);
    freeall(&onbg_list);
    freeall(&stickyNamesList);
    // not neccesary if all plugins properly call 'RemoveSticky':
    freeall(&sticky_list);
}

void Workspaces::Reconfigure ()
{
    bool changed = false;
    SetNames();
    WS_LoadStickyNamesList();
    WS_LoadOnBGNamesList();
    // force reorder on resolution changes
    changed = GetScreenMetrics();
    vwm_reconfig(changed);
}

bool Workspaces::GetScreenMetrics ()
{
    int x, y, w, h;
    if (g_multimon) {
        x = GetSystemMetrics(SM_XVIRTUALSCREEN);
        y = GetSystemMetrics(SM_YVIRTUALSCREEN);
        w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    } else {
        x = y = 0;
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);
    }
    bool const changed = w != VScreenWidth || h != VScreenHeight;
    VScreenWidth = w;
    VScreenHeight = h;
    VScreenX = x;
    VScreenY = y;
    nScreens = Settings_disableVWM ? 1 : Settings_workspaces;
    nScreensX = Settings_workspacesX;
    nScreensY = Settings_workspacesY;
    //dbg_printf("Screen: %d/%d %d/%d", x, y, w, h);
    return changed;
}

//===========================================================================

void Workspaces::SetWorkspaceNames (const char * names)
{
    if (names)
        strcpy(Settings_workspaceNames, names);
    else if (IDOK != EditBox( BBAPPNAME, NLS2("$Workspace_EditNames$", "Workspace Names:"), Settings_workspaceNames, Settings_workspaceNames))
        return;

    Settings_WriteRCSetting(&Settings_workspaceNames);
    SetNames();
    send_desk_refresh();
}

void Workspaces::SetNames ()
{
    freeall(&deskNames);
    char const * names = Settings_workspaceNames;
    for (int i = 0; i < Settings_workspaces; ++i)
    {
        TCHAR wkspc_name[MAX_PATH];
        if (0 == *NextToken(wkspc_name, &names, ","))
            sprintf(wkspc_name,
                NLS2("$Workspace_DefaultName$", "Workspace %d"), i+1);
        append_string_node(&deskNames, wkspc_name);
    }
}

//===========================================================================
HWND Workspaces::get_default_window (HWND hwnd) const
{
    if (NULL == hwnd) {
        hwnd = GetForegroundWindow();
        if (NULL == hwnd || is_bbwindow(hwnd)) {
            hwnd = get_top_window(currentScreen);
        }
    }
    return hwnd;
}

LRESULT send_syscommand (HWND hwnd, WPARAM SC_XXX)
{
    DWORD_PTR dwResult = 0;
    SendMessageTimeout(hwnd, WM_SYSCOMMAND, SC_XXX, 0, SMTO_ABORTIFHUNG|SMTO_NORMAL, 1000, &dwResult);
    return dwResult;
}

DWORD_PTR send_bbls_command (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    DWORD_PTR result = 0;
    SendMessageTimeout(hwnd,
        RegisterWindowMessage(BBLEANSKIN_MSG), wParam, lParam,
        SMTO_ABORTIFHUNG|SMTO_NORMAL, 1000, (DWORD_PTR*)&result);
    return result;
}

void Workspaces::get_desktop_info (DesktopInfo & deskInfo, int i) const
{
    deskInfo.isCurrent = i == currentScreen;
    deskInfo.number = i;
    deskInfo.name[0] = 0;
    deskInfo.nScreens = nScreens;
	deskInfo.nScreensX = nScreensX;
	deskInfo.nScreensY = nScreensY;
    deskInfo.deskNames = deskNames;
    string_node * sp = (string_node *) nth_node(deskNames, i);
    if (sp)
        strcpy(deskInfo.name, sp->m_val);
}

BOOL list_desktops_func(DesktopInfo const * DI, LPARAM lParam)
{
    SendMessage((HWND)lParam, BB_DESKTOPINFO, 0, (LPARAM)DI);
    return TRUE;
}

/*
void post_message_if_needed(UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    if (!PeekMessage(&msg, BBhwnd, message, message, PM_NOREMOVE))
        PostMessage(BBhwnd, message, wParam, lParam);
}
*/

void Workspaces::send_task_message (HWND hwnd, UINT msg) const
{
    SendMessage(BBhwnd, BB_TASKSUPDATE, (WPARAM)hwnd, msg);
}

void Workspaces::send_task_refresh () const
{
    send_task_message(NULL, TASKITEM_REFRESH);
}

void Workspaces::send_desk_refresh () const
{
    DesktopInfo DI;
    g_Workspaces.get_desktop_info(DI, currentScreen);
    SendMessage(BBhwnd, BB_DESKTOPINFO, 0, (LPARAM)&DI);
}

//===========================================================================
LRESULT Workspaces::Command (UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = (HWND)lParam;
    LONG_PTR style;

    switch (msg)
    {
        case BB_SWITCHTON:
            DeskSwitch((int)lParam);
            break;

        case BB_LISTDESKTOPS:
            if (wParam)
                EnumDesks(list_desktops_func, wParam);
            break;

        case BB_MOVEWINDOWTON:
        case BB_SENDWINDOWTON:
            MoveWindowToWkspc(get_default_window(hwnd), (int)wParam, msg == BB_MOVEWINDOWTON);
            break;

        //====================
        case BB_BRINGTOFRONT:
            if (hwnd)
                WS_BringToFront(hwnd, 0 != (wParam & BBBTF_CURRENT));
            else
                FocusTopWindow();
            break;

        //====================
        case BB_WORKSPACE:
            switch (wParam)
            {
                // ---------------------------------
                case BBWS_DESKLEFT:
                    DeskSwitch(NextDesk(-1));
                    break;
                case BBWS_DESKRIGHT:
                    DeskSwitch(NextDesk(1));
                    break;
                case BBWS_DESKDOWN:
                    DeskSwitch(NextDesk(+nScreensX));
                    break;
                case BBWS_DESKUP:
                    DeskSwitch(NextDesk(-nScreensX));
                    break;


                // ---------------------------------
                case BBWS_ADDDESKTOP:
                    AddDesktop(1);
                    break;
                case BBWS_DELDESKTOP:
                    AddDesktop(-1);
                    break;

                // ---------------------------------
                case BBWS_SWITCHTODESK:
                    DeskSwitch((int)lParam);
                    break;

                case BBWS_LASTDESK:
                    DeskSwitch(lastScreen);
                    break;

                case BBWS_GATHERWINDOWS:
                    GatherWindows();
                    break;

                // ---------------------------------
                case BBWS_MOVEWINDOWLEFT:
                    MoveWindowToWkspc(get_default_window(hwnd), NextDesk(-1), true);
                    break;

                case BBWS_MOVEWINDOWRIGHT:
                    MoveWindowToWkspc(get_default_window(hwnd), NextDesk(1), true);
                    break;

                case BBWS_MOVEWINDOWDOWN:
                    MoveWindowToWkspc(get_default_window(hwnd), NextDesk(-nScreensX), true);
                    break;

                case BBWS_MOVEWINDOWUP:
                    MoveWindowToWkspc(get_default_window(hwnd), NextDesk(+nScreensX), true);
                    break;


                // ---------------------------------
                case BBWS_PREVWINDOW:
                    NextWindow(0 != lParam /*true for all workspaces*/, -1);
                    break;

                case BBWS_NEXTWINDOW:
                    NextWindow(0 != lParam /*true for all workspaces*/, 1);
                    break;

                // ---------------------------------
                case BBWS_ISSTICKY:
                    return CheckSticky(hwnd);

                case BBWS_MAKESTICKY:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    MakeSticky(hwnd);
                    break;

                case BBWS_CLEARSTICKY:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    RemoveSticky(hwnd);
                    break;

                case BBWS_TOGGLESTICKY:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    if (CheckSticky(hwnd))
                        RemoveSticky(hwnd);
                    else
                        MakeSticky(hwnd);
                    break;

                case BBWS_TOGGLEONTOP:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    SetWindowPos(hwnd,
                        (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
                        ? HWND_NOTOPMOST : HWND_TOPMOST,
                        0, 0, 0, 0,
                        SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

                    send_bbls_command(hwnd, BBLS_REDRAW, 0);
                    break;

                case BBWS_GETTOPWINDOW:
                    return (LRESULT)get_default_window(hwnd);

                // ---------------------------------
                case BBWS_MAKEONBG:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    MakeOnBG(hwnd);
                    break;

                case BBWS_CLEARONBG:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    RemoveOnBG(hwnd);
                    break;

                case BBWS_TOGGLEONBG:
                    hwnd = get_default_window(hwnd);
                    if (NULL == hwnd)
                        break;
                    if (CheckOnBG(hwnd))
                        RemoveOnBG(hwnd);
                    else
                        MakeOnBG(hwnd);
                    break;

                case BBWS_ISONBG:
                    return CheckOnBG(hwnd);

                // ---------------------------------
                case BBWS_EDITNAME:
                    SetWorkspaceNames((const char *)lParam);
                    break;

                case BBWS_MINIMIZEALL:
                    MinimizeAllWindows();
                    break;

                case BBWS_RESTOREALL:
                    RestoreAllWindows();
                    break;

                case BBWS_CASCADE:
                    CascadeWindows(NULL, 0, NULL, 0, NULL);
                    break;

                case BBWS_TILEVERTICAL:
                    TileWindows(NULL, MDITILE_VERTICAL, NULL, 0, NULL);
                    break;

                case BBWS_TILEHORIZONTAL:
                    TileWindows(NULL, MDITILE_HORIZONTAL, NULL, 0, NULL);
                    break;

                default:
                    break;
            }
            break;

        //====================
        default:
            hwnd = get_default_window(hwnd);
            if (NULL == hwnd)
                break;

            if (currentScreen != vwm_get_desk(hwnd)) {
                if (BB_WINDOWCLOSE == msg)
                    WS_BringToFront(hwnd, true);
                else
                    break;
            }

            style = GetWindowLongPtr(hwnd, GWL_STYLE);
            switch (msg)
            {
                case BB_WINDOWCLOSE:
                    WS_CloseWindow(hwnd);
                    break;

                case BB_WINDOWMINIMIZE:
                    if (0 == (WS_MINIMIZEBOX & style))
                        break;
                    WS_MinimizeWindow(hwnd);
                    break;

                case BB_WINDOWRESTORE:
                    WS_RestoreWindow(hwnd);
                    break;

                case BB_WINDOWMAXIMIZE:
                    if (0 == (WS_MAXIMIZEBOX & style))
                        break;
                    WS_MaximizeWindow(hwnd);
                    break;

                case BB_WINDOWGROWHEIGHT:
                    if (0 == (WS_MAXIMIZEBOX & style))
                        break;
                    WS_GrowWindowHeight(hwnd);
                    break;

                case BB_WINDOWGROWWIDTH:
                    if (0 == (WS_MAXIMIZEBOX & style))
                        break;
                    WS_GrowWindowWidth(hwnd);
                    break;

                case BB_WINDOWLOWER:
                    WS_LowerWindow(hwnd);
                    break;

                case BB_WINDOWRAISE:
                    WS_RaiseWindow(hwnd);
                    break;

                case BB_WINDOWSHADE:
                    if (0 == (WS_SIZEBOX & style))
                        break;
                    WS_ShadeWindow(hwnd);
                    break;

                case BB_WINDOWSIZE:
                    if (0 == (WS_SIZEBOX & style))
                        break;
                    send_syscommand(hwnd, SC_SIZE);
                    break;

                case BB_WINDOWMOVE:
                    send_syscommand(hwnd, SC_MOVE);
                    break;
            }
    }
    return -1;
}

//===========================================================================
void Workspaces::WS_BringToFront(HWND hwnd, bool to_current)
{
    int windesk;

    CleanTasks();

    windesk = vwm_get_desk(hwnd);
    if (windesk != currentScreen)
    {
        if (false == to_current)
            switchToDesktop(windesk);
        else
            setDesktop(hwnd, currentScreen, false);
    }
    SwitchToWindow(hwnd);
}

//===========================================================================

void Workspaces::SwitchToWindow (HWND hwnd_app)
{
    HWND hwnd = GetLastActivePopup(GetRootWindow(hwnd_app));
    if (have_imp(pSwitchToThisWindow)) {
        // this one also restores the window, if it's iconic:
        pSwitchToThisWindow(hwnd, 1);
    } else {
        SetForegroundWindow(hwnd);
        if (IsIconic(hwnd))
            send_syscommand(hwnd, SC_RESTORE);
    }
}

void Workspaces::SwitchToBBWnd () const
{
    ForceForegroundWindow(BBhwnd);
    // sometimes the shell notification doesnt seem to work correctly:
    PostMessage(BBhwnd, g_WM_ShellHook, HSHELL_WINDOWACTIVATED, 0);
}

//===========================================================================

void get_rect (HWND hwnd, RECT * rp)
{
    GetWindowRect(hwnd, rp);
    if (WS_CHILD & GetWindowLongPtr(hwnd, GWL_STYLE))
    {
        HWND pw = GetParent(hwnd);
        ScreenToClient(pw, (LPPOINT)&rp->left);
        ScreenToClient(pw, (LPPOINT)&rp->right);
    }
}

void window_set_pos (HWND hwnd, RECT rc)
{
    int const width = rc.right - rc.left;
    int const height = rc.bottom - rc.top;
    SetWindowPos(hwnd, NULL, rc.left, rc.top, width, height, SWP_NOZORDER|SWP_NOACTIVATE);
}

int get_shade_height (HWND hwnd)
{
    int shade = (int)send_bbls_command(hwnd, BBLS_GETSHADEHEIGHT, 0);
    //dbg_printf("BBLS_GETSHADEHEIGHT: %d", shade);
    if (shade)
        return shade;

    int border = GetSystemMetrics(
        (WS_SIZEBOX & GetWindowLongPtr(hwnd, GWL_STYLE))
        ? SM_CYFRAME
        : SM_CYFIXEDFRAME);

    int caption = GetSystemMetrics(
        (WS_EX_TOOLWINDOW & GetWindowLongPtr(hwnd, GWL_EXSTYLE))
        ? SM_CYSMCAPTION
        : SM_CYCAPTION);

    //dbg_printf("caption %d  border %d", caption, border);
    return 2*border + caption;
}

void Workspaces::WS_ShadeWindow (HWND hwnd)
{
    RECT rc;
    int h1, h2, height;
    HANDLE prop;

    get_rect(hwnd, &rc);
    height = rc.bottom - rc.top;
    prop = GetProp(hwnd, BBSHADE_PROP);

    h1 = LOWORD(prop);
    h2 = HIWORD(prop);
    if (IsZoomed(hwnd)) {
        if (h2) height = h2, h2 = 0;
        else h2 = height, height = get_shade_height(hwnd);
    } else {
        if (h1) height = h1, h1 = 0;
        else h1 = height, height = get_shade_height(hwnd);
        h2 = 0;
    }

    prop = (HANDLE)MAKELPARAM(h1, h2);
    if (0 == prop) RemoveProp(hwnd, BBSHADE_PROP);
    else SetProp(hwnd, BBSHADE_PROP, prop);

    rc.bottom = rc.top + height;
    window_set_pos(hwnd, rc);
}

//===========================================================================
bool check_for_restore (HWND hwnd)
{
    WINDOWPLACEMENT wp;

    if (FALSE == IsZoomed(hwnd))
        return false;
    send_syscommand(hwnd, SC_RESTORE);

    // restore the default maxPos (necessary when it was V-max'd or H-max'd)
    wp.length = sizeof wp;
    GetWindowPlacement(hwnd, &wp);
    wp.ptMaxPosition.x =
    wp.ptMaxPosition.y = -1;
    SetWindowPlacement(hwnd, &wp);
    return true;
}

void grow_window (HWND hwnd, bool v)
{
    RECT r1, r2;

    if (check_for_restore(hwnd))
        return;
    get_rect(hwnd, &r1);
    LockWindowUpdate(hwnd);
    send_syscommand(hwnd, SC_MAXIMIZE);
    get_rect(hwnd, &r2);
    if (v)
        r1.top = r2.top, r1.bottom = r2.bottom;
    else
        r1.left = r2.left, r1.right = r2.right;
    window_set_pos(hwnd, r1);
    LockWindowUpdate(NULL);
}

void Workspaces::WS_GrowWindowHeight (HWND hwnd)
{
    grow_window(hwnd, true);
}

void Workspaces::WS_GrowWindowWidth (HWND hwnd)
{
    grow_window(hwnd, false);
}

void Workspaces::WS_MaximizeWindow (HWND hwnd)
{
    if (check_for_restore(hwnd))
        return;
    send_syscommand(hwnd, SC_MAXIMIZE);
}

void Workspaces::WS_RestoreWindow (HWND hwnd)
{
    if (check_for_restore(hwnd))
        return;
    send_syscommand(hwnd, SC_RESTORE);
}

void Workspaces::WS_MinimizeWindow (HWND hwnd)
{
    if (have_imp(pAllowSetForegroundWindow))
        pAllowSetForegroundWindow(ASFW_ANY);
    send_syscommand(hwnd, SC_MINIMIZE);
}

void Workspaces::WS_CloseWindow (HWND hwnd)
{
    send_syscommand(hwnd, SC_CLOSE);
    PostMessage(BBhwnd, BB_BRINGTOFRONT, 0, 0);
}

void Workspaces::WS_RaiseWindow (HWND hwnd_notused)
{
    tasklist * tl = NULL;
    toptask * lp = 0;
    dolist (lp, pTopTask)
		if (currentScreen == lp->m_val->wkspc)
			tl = lp->m_val;
    if (tl)
		WS_BringToFront(tl->m_val, false);
}

void Workspaces::WS_LowerWindow (HWND hwnd)
{
    tasklist * tl = 0;
    SwitchToBBWnd();
    if (pTopTask) {
		tl = pTopTask->m_val;
        SetTopTask(tl, 2); // append
		if (tl != pTopTask->m_val)
            FocusTopWindow();
    }
    vwm_lower_window(hwnd);
}

//===========================================================================
// API: MakeSticky
// API: RemoveSticky
// API: CheckSticky
// Purpose: make a plugin/app window appear on all workspaces
//===========================================================================

// This is now one API for both plugins and application windows,
// still internally uses different methods

void Workspaces::MakeSticky (HWND hwnd)
{
    StickyNode * p = 0;

    if (FALSE == IsWindow(hwnd))
        return;

    if (is_bbwindow(hwnd)) {
        if (findHwnd(sticky_list, hwnd))
            return;
        p = (StickyNode *)m_alloc(sizeof(StickyNode));
        cons_node(&sticky_list, p);
		p->m_val = hwnd;
        //dbg_window(hwnd, "[+%d]", listlen(sticky_list));
    } else {
        if (vwm_get_desk(hwnd) != currentScreen)
            setDesktop(hwnd, currentScreen, false);
        vwm_set_sticky(hwnd, true);
        send_bbls_command(hwnd, BBLS_SETSTICKY, 1);
        //dbg_window(hwnd, "[+app]");
    }
}

void Workspaces::RemoveSticky (HWND hwnd)
{
    StickyNode **pp = 0, *p = 0;

    pp = (StickyNode **)assoc_ptr(&sticky_list, hwnd);
    if (pp) {

        *pp = (p = *pp)->m_next;
        m_free(p);
        //dbg_window(hwnd, "[-%d]", listlen(sticky_list));

    } else if (vwm_set_sticky(hwnd, false)) {
        send_bbls_command(hwnd, BBLS_SETSTICKY, 0);
        //dbg_window(hwnd, "[-app]");
    }
}
// export to BBVWM.cpp
bool Workspaces::CheckStickyName (HWND hwnd)
{
    string_node * sl = 0;
    char appName[MAX_PATH];
    if (NULL == stickyNamesList || 0 == GetAppByWindow(hwnd, appName))
        return false;
    _strlwr(appName);
    dolist (sl, stickyNamesList)
        if (0==strcmp(appName, sl->m_val))
            return true;
    return false;
}

// export to BBVWM.cpp
bool Workspaces::CheckStickyPlugin (HWND hwnd)
{
    if (findHwnd(sticky_list, hwnd))
        return true;
    // bbPager is the only known plugin that still uses that sticky method:
    if (GetWindowLongPtr(hwnd, GWLP_USERDATA) == 0x49474541 /*magicDWord*/)
        return true;
    return false;
}

bool Workspaces::CheckSticky (HWND hwnd)
{
    return CheckStickyPlugin(hwnd) || vwm_get_status(hwnd, VWM_STICKY);
}

//===========================================================================
void Workspaces::WS_LoadStickyNamesList ()
{
    TCHAR buffer[MAX_PATH];
    FILE * fp = 0;

    freeall(&stickyNamesList);
	tstring path;
    findRcFile(path, "StickyWindows.ini", NULL);
    fp = FileOpen(path);
    if (fp) {
        while (ReadNextCommand(fp, buffer, sizeof (buffer)))
            append_string_node(&stickyNamesList, _strlwr(buffer));
        FileClose(fp);
    }
}

//===========================================================================
// API: MakeOnBG
// API: RemoveOnBG
// API: CheckOnBG
// Purpose: make a plugin/app window appear on all workspaces
//===========================================================================

// This is now one API for both plugins and application windows,
// still internally uses different methods

void Workspaces::MakeOnBG (HWND hwnd)
{
    if (FALSE == IsWindow(hwnd))
        return;

    if (!is_bbwindow(hwnd))
    {
        if (vwm_get_desk(hwnd) != currentScreen)
            setDesktop(hwnd, currentScreen, false);
        vwm_set_onbg(hwnd, true);

        ShowWindow(hwnd, SW_HIDE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW); // hide window from alt-tab tasklist
        ShowWindow(hwnd, SW_SHOW);
        send_bbls_command(hwnd, BBLS_SETONBG, 1);
        //dbg_window(hwnd, "[+app]");
    }
}

void Workspaces::RemoveOnBG (HWND hwnd)
{
    onbg_node **pp = 0, *p = 0;

    pp = (onbg_node**)assoc_ptr(&onbg_list, hwnd);
    if (pp)
    {
        *pp = (p = *pp)->m_next;
        m_free(p);
        //dbg_window(hwnd, "[-%d]", listlen(onbg_list));

    } else if (vwm_set_onbg(hwnd, false)) {

        LONG_PTR const flags = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (WS_EX_TOOLWINDOW & flags)
        {
            ShowWindow(hwnd, SW_HIDE);
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
            ShowWindow(hwnd, SW_SHOW);
        }
        send_bbls_command(hwnd, BBLS_SETONBG, 0);
        //dbg_window(hwnd, "[-app]");
    }
}

// export to BBVWM.cpp
bool Workspaces::check_onbg_plugin (HWND hwnd)
{
    return findHwnd(onbg_list, hwnd) != 0;
}

bool Workspaces::CheckOnBG (HWND hwnd)
{
    return check_onbg_plugin(hwnd) || vwm_get_status(hwnd, VWM_ONBG);
}

void Workspaces::WS_LoadOnBGNamesList ()
{
    freeall(&onBGNamesList);

	tstring path;
    findRcFile(path, TEXT("BGWindows.ini"), NULL);
    FILE * fp = FileOpen(path.c_str());
	char buffer[MAX_PATH];
    if (fp) {
        while (ReadNextCommand(fp, buffer, sizeof (buffer)))
            append_string_node(&onBGNamesList, _strlwr(buffer));
        FileClose(fp);
    }
}

bool Workspaces::CheckOnBgName (HWND hwnd)
{
    string_node * sl = 0;
    char appName[MAX_PATH];
    if (NULL == onBGNamesList || 0 == GetAppByWindow(hwnd, appName))
        return false;
    _strlwr(appName);
    dolist (sl, onBGNamesList)
        if (0==strcmp(appName, sl->m_val))
            return true;
    return false;
}

//===========================================================================
// Functions: Minimize/Restore All Windows
//===========================================================================
struct mr_info
{
    list_node * p;
    int cmd;
    bool iconic;
};

bool Workspaces::mr_checktask (HWND hwnd)
{
    tasklist * tl = (tasklist *)findHwnd(taskList, hwnd);
    return tl && tl->wkspc == currentScreen;
}

BOOL CALLBACK mr_enumproc (HWND hwnd, LPARAM lParam)
{
    if (g_Workspaces.mr_checktask(hwnd))
    {
        mr_info * mr = (mr_info *)lParam;
        if (mr->iconic == (FALSE != IsIconic(hwnd)))
            cons_node (&mr->p, new_node<list_node>(hwnd));
    }
    return TRUE;
}

void Workspaces::min_rest_helper (int cmd)
{
    mr_info mri, *mr = &mri;
    list_node **pp, *p;

    mr->p = NULL;
    mr->iconic = SC_RESTORE == cmd;
    EnumWindows(mr_enumproc, (LPARAM)mr);

    if (SC_RESTORE == cmd)
        reverse_list(&mr->p);

    if (0 == cmd) {
        cmd = SC_MINIMIZE;
        if (NULL == mr->p) {
            for (pp = &toggled_windows; NULL != (p = *pp); ) {
                if (mr_checktask((HWND)p->m_val))
                    *pp = p->m_next, cons_node(&mr->p, p);
                else
                    pp = &p->m_next;
            }
            cmd = SC_RESTORE;
        }
    }

    mr->cmd = cmd;
    freeall(&toggled_windows);
    dolist (p, mr->p) {
        HWND hwnd = (HWND)p->m_val;
        if (SC_MINIMIZE == mr->cmd)
			cons_node(&toggled_windows, new_node<list_node>(hwnd));
        send_syscommand(hwnd, mr->cmd);
    }
    freeall(&mr->p);
}

void Workspaces::MinimizeAllWindows ()
{
    min_rest_helper(0); //SC_MINIMIZE);
}

void Workspaces::RestoreAllWindows ()
{
    min_rest_helper(SC_RESTORE);
}

//================================================================
// get the top window in the z-order of the current workspace

HWND Workspaces::get_top_window (int scrn) const
{
    toptask * lp = 0;
    dolist (lp, pTopTask)
		if (scrn == lp->m_val->wkspc) {
		HWND hwnd = lp->m_val->m_val;
            if (FALSE == IsIconic(hwnd))
                return hwnd;
        }
    return NULL;
}

bool Workspaces::FocusTopWindow ()
{
    HWND hw = get_top_window(currentScreen);
    if (hw) {
        SwitchToWindow(hw);
        return true;
    }
    SwitchToBBWnd();
    return false;
}

//================================================================
// is this window a valid task

int is_valid_task(HWND hwnd)
{
    if (FALSE == IsWindow(hwnd))
        return 0;

    if (IsWindowVisible(hwnd))
        return 1;

    if (Settings_altMethod && vwm_get_status(hwnd, VWM_MOVED))
        return 2;

    return 0;
}

//===========================================================================
// gather windows in current WS
void Workspaces::GatherWindows ()
{
    vwm_gather();
}

//===========================================================================
// the internal switchToDesktop
void Workspaces::switchToDesktop (int n) const
{
    // steel focus and wait for apps to close their menus, because that
    // could leave defunct dropshadows on the screen otherwise
    SwitchToBBWnd();
    BBSleep(10);
    vwm_switch(n);
}

void Workspaces::setDesktop (HWND hwnd, int n, bool switchto) const
{
    vwm_set_desk (hwnd, n, switchto);
}

//===========================================================================
int Workspaces::NextDesk (int d)
{
    int n = currentScreen + d;
    int m = nScreens - 1;

    if (Settings_workspaces_wraparound)
    {
        if (n > m) return 0;
        if (n < 0) return m;
    }
    else
    {
        if (n > m) return currentScreen;
        if (n < 0) return currentScreen;
    }
    return n;
}

void Workspaces::AddDesktop (int d)
{
    Settings_workspaces = imax(1, Settings_workspaces + d);
    Settings_WriteRCSetting(&Settings_workspaces);
    Settings_WriteRCSetting(&Settings_workspacesX);
    Settings_WriteRCSetting(&Settings_workspacesY);
    Reconfigure();
    send_desk_refresh();
}

//====================
void Workspaces::DeskSwitch (int i)
{
    HWND hwnd;

    //dbg_printf("DeskSwitch %d -> %d", currentScreen, i);

    if (i == currentScreen || i < 0 || i >= nScreens)
        return;

    if (activeTaskWindow && vwm_get_status(activeTaskWindow, VWM_STICKY))
        hwnd = activeTaskWindow;
    else
        hwnd = get_top_window(i);

    switchToDesktop(i);
    if (hwnd)
        SwitchToWindow(hwnd);
    else
        SwitchToBBWnd();
}

//====================
void Workspaces::MoveWindowToWkspc (HWND hwnd, int desk, bool switchto)
{
    if (NULL == hwnd)
        return;

    RemoveSticky(hwnd);

    if (switchto) {
        SwitchToWindow(hwnd);
        setDesktop(hwnd, desk, true);

    } else {
        SwitchToBBWnd();
        setDesktop(hwnd, desk, false);
        FocusTopWindow();
    }
}

//====================

void Workspaces::NextWindow (bool allDesktops, int dir)
{
    tasklist *tl;
    int const s = GetTaskListSize();
    if (0==s) return;
	int i = FindTask(pTopTask->m_val->m_val);
    if (-1==i) i=0;
    int const j = i;
    do {
        if (dir>0) {
            i++;
            if (s==i) i=0;
        } else {
            if (0==i) i=s;
            i--;
        }
        tl = (tasklist *)nth_node(taskList, i);
        if (tl && (allDesktops || currentScreen == tl->wkspc)
			&& FALSE == IsIconic(tl->m_val) && FALSE == CheckOnBG(tl->m_val)) {
			PostMessage(BBhwnd, BB_BRINGTOFRONT, 0, (LPARAM)tl->m_val);
            return;
        }
    } while (j!=i);
}

//===========================================================================
void Workspaces::ToggleWindowVisibility(HWND hwnd)
{
    if (IsWindow(hwnd)) {
        ShowWindowAsync(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOWNA);
        send_task_refresh();
    }
}

//===========================================================================

//===========================================================================
// Task - Support

void Workspaces::del_from_toptasks (tasklist * tl)
{
    toptask **lpp, *lp;
    lpp = (toptask**)assoc_ptr(&pTopTask, tl);
    if (lpp)
        *lpp=(lp=*lpp)->m_next, m_free(lp);
}

void Workspaces::SetTopTask (tasklist * tl, int set_where)
{
    if (NULL==tl)
        return;
    del_from_toptasks(tl);
    if (0==set_where)
        return; // delete_only
    if (1==set_where) // push at front
        cons_node(&pTopTask, new_node<toptask>(tl));
    if (2==set_where) // push at end
        append_node(&pTopTask, new_node<toptask>(tl));
}

void Workspaces::get_caption (tasklist * tl, int force)
{
    if (force || 0 == tl->caption[0])
		get_window_text(tl->m_val, tl->caption, sizeof tl->caption);
    if (force || NULL == tl->icon)
		get_window_icon(tl->m_val, &tl->icon);
}

HWND Workspaces::GetActiveTaskWindow ()
{
    return activeTaskWindow;
}

void Workspaces::GetCaptions ()
{
    tasklist *tl;
    dolist (tl, taskList)
        get_caption(tl, 1);
}

//==================================

tasklist * Workspaces::AddTask (HWND hwnd)
{
    tasklist * tl = c_new<tasklist>();
	tl->m_val = hwnd;
    tl->wkspc = currentScreen;
    append_node(&taskList, tl);
    get_caption(tl, 1);
    send_task_message(hwnd, TASKITEM_ADDED);
    return tl;
}

void Workspaces::RemoveTask (tasklist * tl)
{
    HWND hwnd;
    if (tl->icon)
        DestroyIcon(tl->icon);
    del_from_toptasks(tl);
	hwnd = tl->m_val;
    remove_item(&taskList, tl);
    send_task_message(hwnd, TASKITEM_REMOVED);
}

int Workspaces::FindTask (HWND hwnd)
{
    tasklist *tl; int i = 0;
    dolist(tl, taskList) {
		if (tl->m_val == hwnd)
            return i;
        i++;
    }
    return -1;
}

// run through the tasklist and remove invalid tasks
void Workspaces::CleanTasks ()
{
    tasklist **tl = &taskList;
    while (*tl)
		if (is_valid_task((*tl)->m_val))
            tl = &(*tl)->m_next;
        else
            RemoveTask(*tl);
}

//==================================
BOOL CALLBACK TaskProc2 (HWND hwnd, LPARAM lParam)
{
    if (IsAppWindow(hwnd))
        g_Workspaces.SetTopTask(g_Workspaces.AddTask(hwnd), 2);
    return TRUE;
}

void Workspaces::init_tasks ()
{
    EnumWindows(TaskProc2, 0);
}

void Workspaces::exit_tasks ()
{
    while (taskList)
        RemoveTask(taskList);
    freeall(&toggled_windows);
}

//==================================
/* Set workspace number from vwm */
#if 0
void workspaces_set_desk(void)
{
    tasklist *tl;
    dolist (tl, taskList)
        tl->wkspc = vwm_get_desk(tl->hwnd);
}
#else
/* Set workspace number from vwm and reorder the tasklist such that
   tasks on higher workspace come after tasks on lower ones */
void Workspaces::workspaces_set_desk ()
{
    tasklist *tl, *tn, **tpp, *tr = NULL;
    for (tl = taskList; tl; tl = tn) {
		tl->wkspc = vwm_get_desk(tl->m_val);
        for (tpp = &tr; *tpp && (*tpp)->wkspc <= tl->wkspc;)
            tpp = &(*tpp)->m_next;
        tn = tl->m_next, tl->m_next = *tpp, *tpp = tl;
    }
    taskList = tr;
}
#endif

//===========================================================================
#if 0
void debug_tasks(WPARAM wParam, HWND hwnd, int is_task)
{
    static const char * const actions[] = {
        "null", "add", "remove", "activateshell",
        "activate", "minmax", "redraw", "taskman",
        "language", "sysmenu", "endtask", NULL,
        NULL, "replaced", "replacing"
    };

    int n = wParam & 0x7FFF;
    char buffer[MAX_PATH];
    const char *msg = n < array_count(actions) ? actions[n] : NULL;
    if (!msg) msg = "xxx";
    GetAppByWindow(hwnd, buffer);
    dbg_printf("msg %d [%s] hwnd=%x task=%d app=%s desk=%d",
        wParam, msg, hwnd, is_task, buffer, vwm_get_desk(hwnd));
}
#else
#define debug_tasks(a,b,c)
#endif

//===========================================================================
// called from the main windowproc on the registered WM_ShellHook message

void Workspaces::TaskProc (WPARAM wParam, HWND hwnd)
{
    tasklist *tl;
    static HWND hwnd_replacing;

    if (hwnd) {
        tl = (tasklist *)findHwnd(taskList, hwnd);
        if (tl)
            tl->flashing = false;
    } else {
        tl = NULL;
    }

    debug_tasks(wParam, hwnd, NULL != tl);

    switch (wParam & 0x7FFF) {

    //====================
    case HSHELL_WINDOWREPLACING:
        hwnd_replacing = hwnd;
        goto hshell_windowdestroyed;

    case HSHELL_WINDOWREPLACED:
        if (NULL == hwnd_replacing)
            goto hshell_windowdestroyed;
        hwnd = hwnd_replacing;
        hwnd_replacing = NULL;
        if (NULL == tl)
            break;
		tl->m_val = hwnd;
        get_caption(tl, 1);
        if (activeTaskWindow == hwnd)
            goto hshell_windowactivated;
        send_task_message(hwnd, TASKITEM_MODIFIED);
        break;

    //====================
    case HSHELL_WINDOWCREATED: // 1
        // windows reshown by the vwm also trigger the HSHELL_WINDOWCREATED
        if (hwnd && NULL == tl)
        {
            AddTask(hwnd);
            workspaces_set_desk();
        }
        break;

    //====================
    case HSHELL_WINDOWDESTROYED: // 2
    hshell_windowdestroyed:
        // windows hidden by the vwm also trigger the HSHELL_WINDOWDESTROYED
        if (tl && is_valid_task(hwnd) != 2)
        {
            RemoveTask(tl);
        }
        break;

    //====================
    case HSHELL_WINDOWACTIVATED: // 4
    hshell_windowactivated:
    {
        tasklist *tl2;
        HWND prev_fg_window;

        //if (wParam & 0x8000) ...;
        // true if a fullscreen window is present
        // (which is not necessarily this 'hwnd')

        prev_fg_window = activeTaskWindow;
        activeTaskWindow = hwnd;
        dolist (tl2, taskList)
            tl2->active = false;

        if (tl)
        {
            int windesk = vwm_get_desk(hwnd);
            tl->active = true;
            if (currentScreen == windesk)
            {
                // in case the app has windows on other workspaces, this
                // gathers them in the current
                setDesktop(hwnd, currentScreen, false);
            }
            else
            if (Settings_followActive
                && -1 != FindTask(prev_fg_window)
                && FALSE == IsIconic(prev_fg_window))
            {
                // we switch only if there is a previously active window
                // and that window neither was closed nor minimized
                setDesktop(hwnd, windesk, true);
            }
            else
            if (currentScreen == vwm_get_desk(GetLastActivePopup(hwnd)))
            {
                // the app is on other workspaces but has popup'd something
                // (supposedly spontaneously) in the current workspace.
                // We put the popup on the other WS and switch to.
                setDesktop(hwnd, windesk, true);
            }
            else
            {
                // else we prevent the window from being activated
                // and set blackbox as foreground task. We dont
                // want people typing in windows on other workspaces
                SwitchToBBWnd();
                setDesktop(hwnd, windesk, false);
                break;
            }

            // ----------------------------------------
            // insert at head of the 'activated windows' list

            SetTopTask(tl, 1);

            // ----------------------------------------
            // try to grab title & icon, if still missing

            get_caption(tl, 0);
        }

        send_task_message(hwnd, TASKITEM_ACTIVATED);
        break;
    }

    //====================
    case HSHELL_REDRAW: // 6
        if (tl)
        {
            UINT msg = TASKITEM_MODIFIED;
			get_window_text(tl->m_val, tl->caption, sizeof tl->caption);
			get_window_icon(tl->m_val, &tl->icon); // disable for foobar delay issue ?
            if (wParam & 0x8000) {
                msg = TASKITEM_FLASHED;
                tl->flashing = true;
            }
            if (false == tl->active
             && GetForegroundWindow() == GetLastActivePopup(hwnd))
                goto hshell_windowactivated;

            send_task_message(hwnd, msg);
            // post_message_if_needed(BB_TASKSUPDATE, (WPARAM)hwnd, msg);
        }
        break;

    //====================
    case HSHELL_LANGUAGE: // 8 - win9x/me only
        send_task_message(hwnd, TASKITEM_LANGUAGE);
        break;
/*
    //====================
    case HSHELL_TASKMAN: // 7 - win9x/me only
        break;

    case HSHELL_ACTIVATESHELLWINDOW: // 3, never seen
        break;

    case HSHELL_GETMINRECT:          // 5, never seen
        break;

    case HSHELL_ENDTASK: // 10
        break;
*/
    }
}

//===========================================================================
// API: GetTaskListSize - returns the number of currently registered tasks
//===========================================================================

int Workspaces::GetTaskListSize () const
{
    return listlen(taskList);
}

//===========================================================================
// API: GetTaskListPtr - returns the raw task-list
//===========================================================================

tasklist const * Workspaces::GetTaskListPtr () const
{
    return taskList;
}

//===========================================================================
// API: GetTask - returns the HWND of the task by index
//===========================================================================

HWND Workspaces::GetTask (int index) const
{
    tasklist * tl = (tasklist *)nth_node(taskList, index);
	return tl ? tl->m_val : NULL;
}

//===========================================================================
// API: GetActiveTask - returns index of current active task or -1, if none or BB
//===========================================================================

int Workspaces::GetActiveTask () const
{
    tasklist * tl = 0;
    int i = 0;
    dolist (tl, taskList) {
        if (tl->active)
            return i;
        i++;
    }
    return -1;
}

//===========================================================================
// API: GetTaskWorkspace - returns the workspace of the task by HWND
//===========================================================================

int GetTaskWorkspace(HWND hwnd)
{
    return vwm_get_desk(hwnd);
}

//===========================================================================
// API: SetTaskWorkspace - set the workspace of the task by HWND
//===========================================================================

void SetTaskWorkspace(HWND hwnd, int wkspc)
{
    vwm_set_workspace(hwnd, wkspc);
}

//===========================================================================
// API: GetTaskLocation - retrieve the desktop and the original coords for a window
//===========================================================================

bool Workspaces::GetTaskLocation(HWND hwnd, taskinfo * t)
{
    return vwm_get_location(hwnd, t);
}

//===========================================================================
// API: SetTaskLocation - move a window and it's popups to another desktop and/or position
//===========================================================================

bool Workspaces::SetTaskLocation (HWND hwnd, taskinfo const * t, UINT flags)
{
    bool is_top = hwnd == activeTaskWindow;
    if (false == vwm_set_location(hwnd, t, flags))
        return false;
    if (is_top && vwm_get_desk(hwnd) != currentScreen)
        FocusTopWindow();
    return true;
}

//===========================================================================
// API: GetDesktopInfo
//===========================================================================

void Workspaces::GetDesktopInfo (DesktopInfo & deskInfo) const
{
    get_desktop_info(deskInfo, currentScreen);
}

//===========================================================================
// (API:) EnumTasks
//===========================================================================

void Workspaces::EnumTasks (TASKENUMPROC lpEnumFunc, LPARAM lParam)
{
    tasklist * tl = 0;
    dolist (tl, taskList)
        if (FALSE == lpEnumFunc(tl, lParam))
            break;
}

//===========================================================================
// (API:) EnumDesks
//===========================================================================

void EnumDesks (DESKENUMPROC lpEnumFunc, LPARAM lParam)
{
    int n;
    for (n = 0; n < g_Workspaces.GetScreenCount(); n++) {
        DesktopInfo DI;
        g_Workspaces.get_desktop_info(DI, n);
        if (FALSE == lpEnumFunc(&DI, lParam))
            break;
    }
}

//===========================================================================
