/*
 ============================================================================

  This file is part of the bbLean source code.

  Copyright © 2004-2009 grischka
  http://bb4win.sf.net/bblean

  bbLean is free software, released under the GNU General Public License
  (GPL version 2).

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

 ============================================================================
*/
#include "bbPlugin.h"
#include "bblibcompat.h"
#include "StyleStruct.h"
#include "iminmax.h"
#include <blackbox/cmd/Commands.h>
#include <blackbox/bind/bind.h>
//#include "paths.h"
#include <bblib/paths.h>
#include <bblib/utils_paths.h>

//===========================================================================
// API: SetFullTransparency
// Purpose: Wrapper, win9x compatible
// In:      HWND, alpha
// Out:     bool
//===========================================================================

BOOL (WINAPI *qSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD) = NULL;

bool SetFullTransparency(HWND hwnd, BYTE alpha)
{
	HMODULE hUser32=LoadLibraryA("user32.dll");
	if (hUser32)
		qSetLayeredWindowAttributes=(BOOL(WINAPI*)(HWND, COLORREF, BYTE, DWORD))GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    if (NULL == qSetLayeredWindowAttributes) return false;

	LONG_PTR wStyle1 = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	LONG_PTR wStyle2 = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

	//BYTE Alpha = eightScale_up(alpha); // no eightscale
	BYTE Alpha = alpha;
	if (Alpha < 255)
		wStyle2 |= WS_EX_LAYERED;
	else
		wStyle2 &= ~WS_EX_LAYERED;

    if (wStyle2 != wStyle1)
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, wStyle2);

    if (wStyle2 & WS_EX_LAYERED)
		return 0 != qSetLayeredWindowAttributes(hwnd, 0, Alpha, LWA_COLORKEY);

    return true;
}

//===========================================================================
// Function: GetOSVersion - bb4win_mod
// Purpose: Retrieves info about the current OS & bit version
// In: None
// Out: int = Returns an integer indicating the OS & bit version
//===========================================================================

// OSVERSIONINFO osInfo;
// bool         using_NT;
// 
// int GetOSVersion(void)
// {
//     ZeroMemory(&osInfo, sizeof(osInfo));
//     osInfo.dwOSVersionInfoSize = sizeof(osInfo);
//     GetVersionEx(&osInfo);
// 
// 	//64-bit OS test, when running as 32-bit under WoW
// 	BOOL bIs64BitOS= FALSE;
// 	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
// 	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"), L"IsWow64Process");
// 	if (NULL != fnIsWow64Process)
// 		fnIsWow64Process(GetCurrentProcess(), &bIs64BitOS);
// 	/*usingx64 = bIs64BitOS;
// 	//64-bit OS test, if compiled as native 64-bit. In case we ever need it.
// 	if (!usingx64)
// 		usingx64=(sizeof(int)!=sizeof(void*));*/
// 
//     using_NT         = osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;
// 
//     if (using_NT)
// 		return ((osInfo.dwMajorVersion * 10) + osInfo.dwMinorVersion + (bIs64BitOS ? 5 : 0)); // NT 40; Win2kXP 50; Vista 60; etc.
// 
// 
// 	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
//     {
//         if (osInfo.dwMinorVersion >= 90)
//             return 30; // Windows ME
//         if (osInfo.dwMinorVersion >= 10)
//             return 20; // Windows 98
//     }
// 	return 10; // Windows 95
// }





// int parse_bb_version (char const * ver_str, size_t offset)
// {
//     int BBVersion = 0;
//     int a = 0, b = 0, c = 0;
//     if (sscanf(ver_str + offset, "%d.%d.%d", &a, &b, &c) >= 2)
//         BBVersion = a*1000 + b*10 + c;
//     else
//         BBVersion = 2;
//     return BBVersion;
// }
// 
// int BBP_bbversion(void)
// {
//     static int BBVersion = -1;
//     if (-1 == BBVersion)
//     {
//         BBVersion = 0;
//         char const * bbv = GetBBVersion();
// 
//         char const bbLean[] = "bbLean";
//         size_t const bbLean_len = sizeof(bbLean);
//         char const bbZero[] = "BlackboxZero";
//         size_t const bbZero_len = sizeof(bbZero);
// 
//         if (0 == memcmp(bbv, bbLean, bbLean_len - 1))
//             BBVersion = parse_bb_version(bbv, bbLean_len);
//         else if (0 == memcmp(bbv, "bb", 2))
//             BBVersion = 1;
//         else if (0 == memcmp(bbv, bbZero, bbZero_len - 1))
//             BBVersion = parse_bb_version(bbv, bbZero_len);
//     }
//     return BBVersion;
// }

//===========================================================================
// #ifndef BBPLUGIN_NOMENU
// 
// struct _menuitem;
// struct n_menu;
// 
// enum _menuitem_modes
// {
//     i_nop = 1,
//     i_sub ,
//     i_cmd ,
//     i_bol ,
//     i_int ,
//     i_str
// };
// 
// struct n_menu
// {
//     char *title;
//     _menuitem *items;
//     _menuitem **pitems;
//     _menuitem *lastitem;
// 
//     const char *id_string;
//     char **broam_key;
//     bool popup;
// 
//     n_menu(const char *_title)
//     {
//         title = new_str(_title);
//         items = lastitem = NULL;
//         pitems = &items;
//     }
// 
//     ~n_menu()
//     {
//         free_str(&title);
//         delitems();
//     }
// 
//     void additem(_menuitem *mi);
//     void delitems(void);
//     const char *addid(const char *cmd);
//     Menu *convert(char **broam_key, const char *id_string, bool popup);
// };
// 
// //-----------------------------------------------------
// struct _menuitem
// {
//     _menuitem *next;
//     n_menu *menu;
//     char *text;
//     int mode;
//     bool disabled;
// 
//     _menuitem(n_menu *m, const char *_text, int _mode)
//     {
//         mode = _mode;
//         text = new_str(_text);
//         next = NULL;
//         disabled = false;
//         m->additem(this);
//     }
// 
//     virtual ~_menuitem()
//     {
//         free_str(&text);
//     }
// 
//     virtual MenuItem* _make_item(Menu *pMenu)
//     {
//         return NULL;
//     }
// 
// };
// 
// //-----------------------------------------------------
// struct _menuitemnop : _menuitem
// {
//     _menuitemnop(n_menu *m, const char *_text)
//         : _menuitem(m, _text, i_nop)
//     {
//     }
//     MenuItem* _make_item(Menu *pMenu)
//     {
//         return MakeMenuNOP(pMenu, text);
//     }
// };
// 
// //-----------------------------------------------------
// struct _menuitemsub : _menuitem
// {
//     struct n_menu *sub;
//     _menuitemsub(n_menu *m, const char *_text, n_menu* _sub)
//         : _menuitem(m, _text, i_sub)
//     {
//         sub = _sub;
//     }
// 
//     ~_menuitemsub()
//     {
//         delete sub;
//     }
// 
//     MenuItem* _make_item(Menu *pMenu)
//     {
//         char buffer[200];
//         sprintf(buffer, "%s:%s", menu->id_string, sub->title);
//         Menu *s = sub->convert(menu->broam_key, buffer, menu->popup);
//         return MakeSubmenu(pMenu, s, text);
//     }
// };
// 
// //-----------------------------------------------------
// struct _menuitemcmd : _menuitem
// {
//     char *cmd;
//     _menuitemcmd(n_menu *m, const char *_text, const char *_cmd)
//         : _menuitem(m, _text, i_cmd)
//     {
//         cmd = new_str(_cmd);
//     }
//     ~_menuitemcmd()
//     {
//         free_str(&cmd);
//     }
// 
//     MenuItem* _make_item(Menu *pMenu)
//     {
//         return MakeMenuItem(pMenu, text, menu->addid(cmd), false);
//     }
// };
// 
// //-----------------------------------------------------
// struct _menuitembol : _menuitemcmd
// {
//     bool checked;
//     _menuitembol(n_menu *m, const char *_text, const char *_cmd, bool _checked)
//         : _menuitemcmd(m, _text, _cmd)
//     {
//         checked = _checked;
//         mode = i_bol;
//     }
//     MenuItem* _make_item(Menu *pMenu)
//     {
//         return MakeMenuItem(pMenu, text, menu->addid(cmd), checked);
//     }
// };
// 
// //-----------------------------------------------------
// struct _menuitemstr : _menuitem
// {
//     char *cmd;
//     char *init;
//     _menuitemstr(n_menu *m, const char *_text, const char *_cmd, const char *_init)
//         : _menuitem(m, _text, i_str)
//     {
//         cmd = new_str(_cmd);
//         init = new_str(_init);
//     }
//     ~_menuitemstr()
//     {
//         free_str(&cmd);
//         free_str(&init);
//     }
//     MenuItem* _make_item(Menu *pMenu)
//     {
//         return MakeMenuItemString(pMenu, text, menu->addid(cmd), init);
//     }
// };
// 
// //-----------------------------------------------------
// struct _menuitemint : _menuitem
// {
//     char *cmd;
//     int initval;
//     int minval;
//     int maxval;
//     _menuitemint(n_menu *m, const char *_text, const char *_cmd, int _initval, int _minval, int _maxval)
//     : _menuitem(m, _text, i_int)
//     {
//         cmd = new_str(_cmd);
//         initval = _initval;
//         minval = _minval;
//         maxval = _maxval;
//     }
//     ~_menuitemint()
//     {
//         free_str(&cmd);
//     }
//     MenuItem* _make_item(Menu *pMenu)
//     {
//         if (BBVERSION_09X) {
//             char buffer[20]; sprintf(buffer, "%d", initval);
//             return MakeMenuItemString(pMenu, text, menu->addid(cmd), buffer);
//         } else {
//             return MakeMenuItemInt(pMenu, text, menu->addid(cmd), initval, minval, maxval);
//         }
//     }
// };
// 
// //-----------------------------------------------------
// void n_menu::additem(_menuitem *mi)
// {
//     *pitems = mi;
//     pitems = &mi->next;
//     mi->menu = this;
//     lastitem = mi;
// }
// 
// void n_menu::delitems(void)
// {
//     _menuitem *mi = items;
//     while (mi)
//     {
//         _menuitem *n = mi->next;
//         delete mi;
//         mi = n;
//     }
// }
// 
// const char *n_menu::addid(const char *cmd)
// {
//     if ('@' == *cmd) return cmd;
//     strcpy(this->broam_key[1], cmd);
//     return this->broam_key[0];
// }
// 
// Menu *n_menu::convert(char **broam_key, const char *_id_string, bool _popup)
// {
//     this->id_string = _id_string;
//     this->popup = _popup;
//     this->broam_key = broam_key;
//     //dbg_printf("<%s>", id_string);
//     Menu *pMenu = MakeNamedMenu(title, id_string, popup);
//     _menuitem *mi = items; while (mi)
//     {
//         MenuItem *pItem = mi->_make_item(pMenu);
//         if (mi->disabled)
//             MenuItemOption(pItem, BBMENUITEM_DISABLED);
//         mi = mi->next;
//     }
//     return pMenu;
// }
// 
// //-----------------------------------------------------
// n_menu *n_makemenu(const char *title)
// {
//     return new n_menu(title);
// }
// 
// n_menu *n_submenu(n_menu *m, const char *text)
// {
//     n_menu *s = n_makemenu(text);
//     new _menuitemsub(m, text, s);
//     return s;
// }
// 
// void n_menuitem_nop(n_menu *m, const char *text)
// {
//     new _menuitemnop(m, text);
// }
// 
// void n_menuitem_cmd(n_menu *m, const char *text, const char *cmd)
// {
//     new _menuitemcmd(m, text, cmd);
// }
// 
// void n_menuitem_bol(n_menu *m, const char *text, const char *cmd, bool check)
// {
//     new _menuitembol(m, text, cmd, check);
// }
// 
// void n_menuitem_int(n_menu *m, const char *text, const char *cmd, int val, int vmin, int vmax)
// {
//     new _menuitemint(m, text, cmd, val, vmin, vmax);
// }
// 
// void n_menuitem_str(n_menu *m, const char *text, const char *cmd, const char *init)
// {
//     new _menuitemstr(m, text, cmd, init);
// }
// 
// void n_disable_lastitem(n_menu *m)
// {
//     if (m->lastitem)
//         m->lastitem->disabled = true;
// }
// 
// Menu *n_convertmenu(n_menu *m, const char *broam_key, bool popup)
// {
//     char broam[200];
//     int x = sprintf(broam, "@%s.", broam_key);
//     char *b[2] = { broam, broam+x };
//     Menu *pMenu = m->convert(b, broam_key, popup);
//     delete m;
//     return pMenu;
// }
// 
// void BBP_showmenu(plugin_info *PI, Menu *pMenu, int flags)
// {
//     if (flags & (int)true) {
//         HWND hwnd = PI->inSlit?PI->hSlit:PI->hwnd;
//         MenuOption(pMenu, (flags & ~(int)true) | BBMENU_HWND, PI->hwnd);
//         PostMessage(hwnd, BB_AUTOHIDE, 1, 1);
//     }
//     ShowMenu(pMenu);
// }
// 
// void n_showmenu(plugin_info *PI, n_menu *m, bool popup, int flags, ...)
// {
//     Menu *pMenu;
//     pMenu = n_convertmenu(m, PI->broam_key, popup);
//     if (flags) {
//         va_list vl;
//         va_start(vl, flags);
//         void *o1, *o2, *o3;
//         o1 = va_arg(vl, void*);
//         o2 = va_arg(vl, void*);
//         o3 = va_arg(vl, void*);
//         MenuOption(pMenu, flags, o1, o2, o3);
//     }
//     BBP_showmenu(PI, pMenu, popup);
// }
// 
// #endif // def BBPLUGIN_NOMENU

//*****************************************************************************

//*****************************************************************************

int get_place(plugin_info *PI)
{
    int sw, sh, x, y, w, h;
    bool top, vcenter, bottom, left, center, right;

    PI->hMon = GetMonitorRect(PI->hwnd, &PI->mon_rect, GETMON_FROM_WINDOW);

    sw  = PI->mon_rect.right  - PI->mon_rect.left;
    sh  = PI->mon_rect.bottom - PI->mon_rect.top;
    x   = PI->xpos - PI->mon_rect.left;
    y   = PI->ypos - PI->mon_rect.top;
    w   = PI->width;
    h   = PI->height;

    x = iminmax(x, 0, sw - w);
    y = iminmax(y, 0, sh - h);

    top        = y == 0;
    vcenter    = y == sh/2 - h/2;
    bottom     = y == (sh - h);

    left       = x == 0;
    center     = x == sw/2 - w/2;
    right      = x == (sw - w);

    if (top)
    {
        if (left)   return POS_TopLeft   ;
        if (center) return POS_TopCenter ;
        if (right)  return POS_TopRight  ;
        return POS_Top;
    }

    if (bottom)
    {
        if (left)   return POS_BottomLeft   ;
        if (center) return POS_BottomCenter ;
        if (right)  return POS_BottomRight  ;
        return POS_Bottom;
    }

    if (left)
    {
        if (vcenter) return POS_CenterLeft;
        return POS_Left;
    }

    if (right)
    {
        if (vcenter) return POS_CenterRight;
        return POS_Right;
    }

    if (center)
    {
        if (vcenter) return POS_Center;
        return POS_CenterH;
    }

    if (vcenter)
        return POS_CenterV;
    return POS_User;
}

//===========================================================================

void set_place(plugin_info *PI)
{
    int sw, sh, x, y, w, h, place;

    sw  = PI->mon_rect.right  - PI->mon_rect.left;
    sh  = PI->mon_rect.bottom - PI->mon_rect.top;
    x   = PI->xpos - PI->mon_rect.left;
    y   = PI->ypos - PI->mon_rect.top;
    w   = PI->width;
    h   = PI->height;
    place = PI->place;

    switch (place)
    {

        case POS_Top:
        case POS_TopLeft:
        case POS_TopCenter:
        case POS_TopRight:
            y = 0;
            break;

        case POS_CenterLeft:
        case POS_CenterRight:
        case POS_CenterV:
        case POS_Center:
            y = sh/2 - h/2;
            break;

        case POS_Bottom:
        case POS_BottomLeft:
        case POS_BottomCenter:
        case POS_BottomRight:
            y = sh - h;
            break;
    }

    switch (place)
    {
        case POS_Left:
        case POS_TopLeft:
        case POS_CenterLeft:
        case POS_BottomLeft:
            x = 0;
            break;

        case POS_TopCenter:
        case POS_BottomCenter:
        case POS_CenterH:
        case POS_Center:
        x = sw/2 - w/2;
            break;

        case POS_Right:
        case POS_TopRight:
        case POS_CenterRight:
        case POS_BottomRight:
            x = sw - w;
            break;
    }

    PI->xpos = PI->mon_rect.left + iminmax(x, 0, sw - w);
    PI->ypos = PI->mon_rect.top  + iminmax(y, 0, sh - h);
}

//===========================================================================

void BBP_set_window_modes(plugin_info *PI)
{
    bool useslit = PI->useSlit && PI->hSlit;
    bool visible = PI->visible && (false == PI->toggled_hidden || useslit);
    bool updateslit = false;
    BYTE trans = 255;

    PI->auto_hidden = false;

    if (visible != PI->is_visible)
    {
        ShowWindow(PI->hwnd, visible ? SW_SHOWNA : SW_HIDE);
        PI->is_visible = visible;
        updateslit = true;
    }

    if (useslit)// && false == hidden)
    {
        RECT r;
        int w, h;
        bool update_size;

        GetWindowRect(PI->hwnd, &r);
        w = r.right - r.left;
        h = r.bottom - r.top;
        update_size = (w != PI->width || h != PI->height);

        if (update_size)
        {
            SetWindowPos(
                PI->hwnd,
                NULL,
                0,
                0,
                PI->width,
                PI->height,
                SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER
                );
            updateslit = true;
        }

        if (false == PI->inSlit)
        {
            if (PI->is_alpha != trans)
            {
                SetTransparency(PI->hwnd, trans);
                PI->is_alpha = trans;
            }
            PI->inSlit = true;
						wchar_t const * const brkey = PI->broam_key;
            SendMessage(PI->hSlit, SLIT_ADD, (WPARAM)brkey, (LPARAM)PI->hwnd);
        }
        else
        if (updateslit)
        {
            SendMessage(PI->hSlit, SLIT_UPDATE, 0, (LPARAM)PI->hwnd);
        }
    }
    else
    {
        int x, y, w, h;
        HWND hwnd_after;
        UINT flags = SWP_NOACTIVATE;

        if (PI->inSlit)
        {
            SendMessage(PI->hSlit, SLIT_REMOVE, 0, (LPARAM)PI->hwnd);
            PI->inSlit = false;
        }

        set_place(PI);
        x = PI->xpos;
        y = PI->ypos;
        w = PI->width;
        h = PI->height;

        if (PI->autoHide && false == PI->auto_shown)
        {
            int hangout = 3;
            int place = PI->place;
            if ((false == PI->orient_vertical && (place == POS_TopLeft || place == POS_BottomLeft))
                || place == POS_CenterLeft || place == POS_Left)
            {
                x = PI->mon_rect.left, w = hangout;
                PI->auto_hidden = true;
            }
            else
            if ((false == PI->orient_vertical && (place == POS_TopRight || place == POS_BottomRight))
                || place == POS_CenterRight || place == POS_Right)
            {
                x = PI->mon_rect.right - hangout, w = hangout;
                PI->auto_hidden = true;
            }
            else
            if (place == POS_TopLeft || place == POS_TopCenter || place == POS_TopRight || place == POS_Top)
            {
                y = PI->mon_rect.top, h = hangout;
                PI->auto_hidden = true;
            }
            else
            if (place == POS_BottomLeft || place == POS_BottomCenter || place == POS_BottomRight || place == POS_Bottom)
            {
                y = PI->mon_rect.bottom - hangout, h = hangout;
                PI->auto_hidden = true;
            }
        }

        if (WS_CHILD & GetWindowLongPtr(PI->hwnd, GWL_STYLE))
        {
            flags |= SWP_NOZORDER;
            hwnd_after = NULL;
        }
        else
        {
            if (PI->alwaysOnTop || PI->auto_shown || PI->auto_hidden)
                hwnd_after = HWND_TOPMOST;
            else
                hwnd_after = HWND_NOTOPMOST;
        }

        SetWindowPos(PI->hwnd, hwnd_after, x, y, w, h, flags);

        if (PI->auto_hidden)
            trans = 16;
        else
        if (PI->alphaEnabled)
            trans = PI->alphaValue;

        if (PI->is_alpha != trans)
        {
            if (PI->alphaEnabled)
				SetFullTransparency(PI->hwnd, trans);
			else
				SetTransparency(PI->hwnd, trans);
            PI->is_alpha = trans;
        }
    }

    PI->pos_changed();
}

//===========================================================================
const wchar_t * const placement_strings[] = {
	TEXT("User")          ,

	TEXT("TopLeft")       ,
	TEXT("TopCenter")     ,
	TEXT("TopRight")      ,

	TEXT("BottomLeft")    ,
	TEXT("BottomCenter")  ,
	TEXT("BottomRight")   ,

	TEXT("CenterLeft")    ,
	TEXT("CenterRight")   ,
	TEXT("Center")        ,

	TEXT("Top")           ,
	TEXT("Bottom")        ,
	TEXT("Left")          ,
	TEXT("Right")         ,
	TEXT("CenterH") ,
	TEXT("CenterV") ,
  NULL
};

const wchar_t * const menu_placement_strings[] = {
  TEXT("User")          ,

	TEXT("Top Left")       ,
  TEXT("Top Center")     ,
  TEXT("Top Right")      ,

	TEXT("Bottom Left")    ,
	TEXT("Bottom Center")  ,
	TEXT("Bottom Right")   ,

	TEXT("Center Left")    ,
	TEXT("Center Right")   ,
	TEXT("Center Screen")  ,

	TEXT("Top")           ,
	TEXT("Bottom")        ,
	TEXT("Left")          ,
	TEXT("Right")         ,
	TEXT("CenterH") ,
	TEXT("CenterV") ,
  NULL
};

//===========================================================================
const wchar_t * BBP_placement_string (int pos)
{
  if (pos < 0 || pos >= POS_LAST)
		pos = 0;
  return placement_strings[pos];
}

int BBP_get_placement (const wchar_t * place_string)
{
    return get_string_index(place_string, placement_strings);
}

//===========================================================================
#ifndef BBPLUGIN_NOMENU

// static char *make_key(char *buffer, struct plugin_info *PI, const char *rcs)
// {
//     const char *s;
//     char *d = buffer;
//     s = PI->rc_key;
// 		while (0 != (*d = *s)) 
// 			++s, ++d;
//     *d++ = '.';
//     s = rcs; while (0 != (*d = *s)) ++s, ++d;
//     *d++ = ':'; *d = 0;
//     return buffer;
// }

static wchar_t *make_key (wchar_t *buffer, size_t buff_sz, plugin_info *PI, const wchar_t *rcs)
{
	_snwprintf_s(buffer, buff_sz, _TRUNCATE, L"%s.%s", PI->rc_key, rcs);
  return buffer;
}

void BBP_write_string(struct plugin_info *PI, const char *rcs, const char *val)
{
//     char buffer[256];
//     if (PI->rc_key)
//         WriteString(PI->rcpath, make_key(buffer, PI, rcs), val);
}

void BBP_write_int(struct plugin_info *PI, const char *rcs, int val)
{
//     char buffer[256];
//     if (PI->rc_key)
//         WriteInt(PI->rcpath, make_key(buffer, PI, rcs), val);
}

void BBP_write_bool(struct plugin_info *PI, const char *rcs, bool val)
{
//     char buffer[256];
//     if (PI->rc_key)
//         WriteBool(PI->rcpath, make_key(buffer, PI, rcs), val);
}

void BBP_write_string(struct plugin_info *PI, const wchar_t *rcs, const wchar_t *val)
{
	//     char buffer[256];
	//     if (PI->rc_key)
	//         WriteString(PI->rcpath, make_key(buffer, PI, rcs), val);
}

void BBP_write_int(struct plugin_info *PI, const wchar_t *rcs, int val)
{
	//     char buffer[256];
	//     if (PI->rc_key)
	//         WriteInt(PI->rcpath, make_key(buffer, PI, rcs), val);
}

void BBP_write_bool(struct plugin_info *PI, const wchar_t *rcs, bool val)
{
	//     char buffer[256];
	//     if (PI->rc_key)
	//         WriteBool(PI->rcpath, make_key(buffer, PI, rcs), val);
}


// 
const char* BBP_read_string(struct plugin_info *PI, char *dest, const char *rcs, const char *def)
{
	TODO_ASSERT(0, "missing ansi version");
//     char buffer[256];
//     const char *p = ReadString(PI->rcpath, make_key(buffer, PI, rcs), def);
//     if (p && dest) return strcpy(dest, p);
//     return p;
	return nullptr;
}

int BBP_read_int(struct plugin_info *PI, const char *rcs, int def)
{
	TODO_ASSERT(0, "missing ansi version");
//     char buffer[256];
//     return ReadInt(PI->rcpath, make_key(buffer, PI, rcs), def);
	return 0;
}

bool BBP_read_bool(struct plugin_info *PI, const char *rcs, bool def)
{
	TODO_ASSERT(0, "missing ansi version");
//     char buffer[256];
//     return ReadBool(PI->rcpath, make_key(buffer, PI, rcs), def);
	return false;;
}
// 
// 
// const char * BBP_read_value(struct plugin_info *PI, const char *rcs, LONG *pPos)
// {
//     char buffer[256];
//     return ReadValue(PI->rcpath, make_key(buffer, PI, rcs), pPos);
// }
bool BBP_read_string(struct plugin_info *PI, wchar_t *dest, size_t const dest_sz, const wchar_t *rcs, const wchar_t *def)
{
	wchar_t buffer[256];
	const wchar_t *p = ReadString(PI->rcpath, make_key(buffer, 256, PI, rcs), def);
	if (p && dest)
	{
		_tcsncpy_s(dest, dest_sz, p, _TRUNCATE);
		return true;
	}
	return false;
}

int BBP_read_int(struct plugin_info *PI, const wchar_t *rcs, int def)
{
	wchar_t buffer[256];
	return ReadInt(PI->rcpath, make_key(buffer, 256, PI, rcs), def);
}

bool BBP_read_bool(struct plugin_info *PI, const wchar_t *rcs, bool def)
{
	wchar_t buffer[256];
	return ReadBool(PI->rcpath, make_key(buffer, 256, PI, rcs), def);
}


// 
// void BBP_rename_setting(struct plugin_info *PI, const char *rcs, const char *rcnew)
// {
// //     char buffer1[256], buffer2[256];
// //     RenameSetting(PI->rcpath, make_key(buffer1, PI, rcs),
// //         rcnew ? make_key(buffer2, PI, rcnew) : NULL);
// }
// 
// void write_rc(struct plugin_info *PI, void *v)
// {
//     if (NULL == PI->rc_key)
//         return;
// 
//     if (v == &PI->xpos)
//     {
//         BBP_write_string    (PI, "placement", BBP_placement_string(PI->place));
//         BBP_write_int       (PI, "position.x", PI->xpos);
//         BBP_write_int       (PI, "position.y", PI->ypos);
//     }
//     else
//     if (v == &PI->width)
//     {
//         BBP_write_int       (PI, "width",  PI->width);
//         BBP_write_int       (PI, "height", PI->height);
//     }
//     else
//     if (v == &PI->useSlit)
//         BBP_write_bool      (PI, "useSlit", PI->useSlit);
//     else
//     if (v == &PI->alwaysOnTop)
//         BBP_write_bool      (PI, "alwaysOnTop", PI->alwaysOnTop);
//     else
//     if (v == &PI->autoHide)
//         BBP_write_bool      (PI, "autoHide", PI->autoHide);
//     else
//     if (v == &PI->clickRaise)
//         BBP_write_bool      (PI, "clickRaise", PI->clickRaise);
//     else
//     if (v == &PI->snapWindow)
//         BBP_write_int      (PI, "snapWindow", PI->snapWindow);
//     else
//     if (v == &PI->pluginToggle)
//         BBP_write_bool      (PI, "pluginToggle", PI->pluginToggle);
//     else
//     if (v == &PI->alphaEnabled)
//         BBP_write_bool      (PI, "alpha.enabled", PI->alphaEnabled);
//     else
//     if (v == &PI->alphaValue)
//         BBP_write_int       (PI, "alpha.value", PI->alphaValue);
//     else
// 	if (v == &PI->saturation)
// 		BBP_write_int       (PI, "icon.saturation", PI->saturation);
// 	else
// 	if (v == &PI->hue)
// 		BBP_write_int       (PI, "icon.hue", PI->hue);
// 	else
//     if (v == &PI->orient_vertical)
//         BBP_write_string    (PI, "orientation", PI->orient_vertical ? "vertical" : "horizontal");
// }

//===========================================================================

bool BBP_read_window_modes(struct plugin_info *PI, const char *rcfile)
{
//     if (PI->rcpath.empty())
// 		{
// 			char tmp[1024];
//       BBP_get_rcpath(tmp, PI->hInstance, rcfile);
// 
// 			TODO_ASSERT(0, "missing conversion");
// 			//PI->rcpath = bbstring(tmp);
// 		}
// 
//     PI->xpos = BBP_read_int(PI,  "position.x", 20);
//     PI->ypos = BBP_read_int(PI,  "position.y", 20);
// 
//     const char *place_string = BBP_read_string(PI, NULL, "placement", NULL);
//     if (place_string)
//         PI->place = BBP_get_placement(place_string);
// 
//     PI->hMon = GetMonitorRect(&PI->xpos, &PI->mon_rect, GETMON_FROM_POINT);
//     set_place(PI);
// 
//     PI->useSlit         = BBP_read_bool(PI, "useSlit", false);
//     PI->alwaysOnTop     = BBP_read_bool(PI, "alwaysOnTop", false);
//     PI->autoHide        = BBP_read_bool(PI, "autoHide", false);
//     PI->snapWindow      = BBP_read_int(PI, "snapWindow", 20);
//     PI->pluginToggle    = BBP_read_bool(PI, "pluginToggle", true);
//     PI->clickRaise      = BBP_read_bool(PI, "clickRaise", true);
//     PI->alphaEnabled    = BBP_read_bool(PI, "alpha.enabled", false);
//     PI->alphaValue      = (BYTE)BBP_read_int(PI,  "alpha.value",  192);
//     //PI->alphaValue      = (BYTE)eightScale_up(BBP_read_int(PI,  "alpha.value",  *(int *)GetSettingPtr(SN_MENUALPHA))); // bb4win
//     PI->orient_vertical  = PI->is_bar || 0 == _stricmp("vertical", BBP_read_string(PI, NULL, "orientation", "vertical"));
//     if (false == PI->no_icons)
//     {
//         //PI->saturation      = eightScale_up(BBP_read_int(PI,  "icon.saturation", 3));
//         //PI->hue             = eightScale_up(BBP_read_int(PI,  "icon.hue", 2));
//         PI->saturation      = BBP_read_int(PI,  "icon.saturation", 80);
//         PI->hue             = BBP_read_int(PI,  "icon.hue", 60);
//     }
//     if (NULL == place_string) {
//         BBP_write_window_modes(PI);
//         return false;
//     }
//     return true;
	return false;
}

bool BBP_read_window_modes(struct plugin_info *PI, const wchar_t * rcfile)
{
	if (PI->rcpath[0] == L'\0')
		BBP_get_rcpath(PI->rcpath, MAX_PATH, PI->hInstance, rcfile);

	PI->xpos = BBP_read_int(PI, L"position.x", 20);
	PI->ypos = BBP_read_int(PI, L"position.y", 20);

	wchar_t place_string[256];
	if (BBP_read_string(PI, place_string, 256, L"placement", NULL))
		PI->place = BBP_get_placement(place_string);

	PI->hMon = GetMonitorRect(&PI->xpos, &PI->mon_rect, GETMON_FROM_POINT);
	set_place(PI);

	PI->useSlit = BBP_read_bool(PI, L"useSlit", false);
	PI->alwaysOnTop = BBP_read_bool(PI, L"alwaysOnTop", false);
	PI->autoHide = BBP_read_bool(PI, L"autoHide", false);
	PI->snapWindow = BBP_read_int(PI, L"snapWindow", 20);
	PI->pluginToggle = BBP_read_bool(PI, L"pluginToggle", true);
	PI->clickRaise = BBP_read_bool(PI, L"clickRaise", true);
	PI->alphaEnabled = BBP_read_bool(PI, L"alpha.enabled", false);
	PI->alphaValue = (BYTE)BBP_read_int(PI, L"alpha.value", 192);
	//PI->alphaValue      = (BYTE)eightScale_up(BBP_read_int(PI,  "alpha.value",  *(int *)GetSettingPtr(SN_MENUALPHA))); // bb4win
	wchar_t const * tmp_orient = ReadString(PI->rcpath, L"orientation", L"vertical");
	PI->orient_vertical = PI->is_bar || 0 == _tcsicmp(L"vertical", tmp_orient);
	if (false == PI->no_icons)
	{
		//PI->saturation      = eightScale_up(BBP_read_int(PI,  "icon.saturation", 3));
		//PI->hue             = eightScale_up(BBP_read_int(PI,  "icon.hue", 2));
		PI->saturation = BBP_read_int(PI, L"icon.saturation", 80);
		PI->hue = BBP_read_int(PI, L"icon.hue", 60);
	}
	if (NULL == place_string) {
		BBP_write_window_modes(PI);
		return false;
	}
	return true;
}

void BBP_write_window_modes(struct plugin_info *PI)
{
//     write_rc(PI, &PI->xpos);
//     write_rc(PI, &PI->useSlit);
//     write_rc(PI, &PI->alwaysOnTop);
//     write_rc(PI, &PI->autoHide);
//     write_rc(PI, &PI->clickRaise);
//     write_rc(PI, &PI->snapWindow);
//     write_rc(PI, &PI->pluginToggle);
//     write_rc(PI, &PI->alphaEnabled);
//     write_rc(PI, &PI->alphaValue);
//     if (false == PI->no_icons)
//     {
//         write_rc(PI, &PI->saturation);
//         write_rc(PI, &PI->hue);
//     }
//     if (false == PI->is_bar)
//         write_rc(PI, &PI->orient_vertical);
}

//===========================================================================

//===========================================================================

// n_menu * BBP_n_placementmenu(struct plugin_info *PI, n_menu *m)
// {
//     n_menu *P; int n, last;
//     P = n_submenu(m, "Placement");
//     last = PI->is_bar ? POS_BottomRight : POS_Center;
//     for (n = 1; n <= last; n++)
//     {
//         if (POS_BottomLeft == n || POS_CenterLeft == n)
//             n_menuitem_nop(P, NULL);
// 
//         char b2[80];
//         sprintf(b2, "placement %s", placement_strings[n]);
//         n_menuitem_bol(P, menu_placement_strings[n], b2, PI->place == n);
//     }
//     return P;
// }
// 
// n_menu * BBP_n_orientmenu(struct plugin_info *PI, n_menu *m)
// {
//     n_menu *o = n_submenu(m, "Orientation");
//     n_menuitem_bol(o, "Vertical", "orientation vertical",  false != PI->orient_vertical);
//     n_menuitem_bol(o, "Horizontal", "orientation horizontal",  false == PI->orient_vertical);
//     return o;
// }
// 
// void BBP_n_insertmenu(struct plugin_info *PI, n_menu *m)
// {
//     if (PI->hSlit)
//     {
//         n_menuitem_bol(m, "Use Slit", "useSlit", PI->useSlit);
//     }
// 
//     if (false == PI->no_icons)
//     {
//         n_menuitem_int(m, "Icon Saturation", "icon.saturation", PI->saturation, 0, 255);
//         n_menuitem_int(m, "Icon Hue", "icon.hue",  PI->hue, 0, 255);
//         //n_menuitem_int(m, "Icon Saturation", "icon.saturation",  eightScale_down(PI->saturation), 0, 8);
//         //n_menuitem_int(m, "Icon Hue", "icon.hue",  eightScale_down(PI->hue), 0, 8);
//     }
// }
// 
// n_menu * BBP_n_windowmenu (plugin_info * PI, n_menu * m)
// {
//      n_menu * R = n_submenu(m, "Window");
// 
//     if (false == PI->useSlit || NULL == PI->hSlit)
//     {
//         n_menuitem_bol(m, "Always On Top", "alwaysOnTop",  PI->alwaysOnTop);
//         n_menuitem_bol(m, "Auto Hide", "autoHide",  PI->autoHide);
//         if (false == PI->alwaysOnTop)
//             n_menuitem_bol(m, "Raise on DeskClick", "clickRaise", PI->clickRaise);
//         n_menuitem_int(R, "Snap To Edge", "snapWindow",  PI->snapWindow, 0, 50);
//         n_menuitem_bol(m, "Toggle With Plugins", "pluginToggle",  PI->pluginToggle);
//         n_menuitem_nop(m, NULL);
//         n_menuitem_bol(m, "Transparency", "alpha.enabled",  PI->alphaEnabled);
//         //n_menuitem_int(R, "Alpha Value", "alpha.value",  eightScale_down(PI->alphaValue), 0, 8); // bb4win
//         n_menuitem_int(m, "Alpha Value", "alpha.value",  PI->alphaValue, 0, 255);
//     }
//     // n_menuitem_bol(m, "Visible", "visible",  PI->visible);
//     return R;
// }

//===========================================================================
#else // BBPLUGIN_NOMENU
//===========================================================================

#define write_rc(a,b)
#define BBP_handle_broam(a,b) 0
static void pos_changed(plugin_info* PI) {}
static void process_broam(plugin_info* PI, const char *broam, int f) {}
plugin_info *BBP_create_info(void)
{
    plugin_info *PI;
    PI = (plugin_info *)m_alloc(sizeof (plugin_info));
    memset(PI, 0, sizeof(plugin_info));
    PI->pos_changed = pos_changed;
    PI->process_broam = process_broam;
    return PI;
}

#endif // def BBPLUGIN_NOMENU

//===========================================================================

void BBP_set_visible(plugin_info *PI, bool visible)
{
    if (PI->visible != visible)
    {
        PI->visible = visible;
        BBP_set_window_modes(PI);
    }
}

void BBP_exit_moving(plugin_info *PI)
{
    if (false == PI->inSlit && (PI->is_moving || PI->is_sizing))
    {
        RECT r;
        GetWindowRect(PI->hwnd, &r);

        if (PI->is_sizing) {
            PI->width = r.right - r.left;
            PI->height = r.bottom - r.top;
//            write_rc(PI, &PI->width);
        } else {
            PI->xpos = r.left;
            PI->ypos = r.top;
            PI->place = get_place(PI);
//            write_rc(PI, &PI->xpos);
        }
    }

    PI->is_moving = false;
    PI->is_sizing = false;
}

// void BBP_set_place(plugin_info *PI, int n)
// {
//     PI->place = n;
//     BBP_set_window_modes(PI);
//     write_rc(PI, &PI->xpos);
// }
// 
// void BBP_set_size(plugin_info *PI, int w, int h)
// {
//     if (PI->width != w || PI->height != h)
//     {
//         PI->width = w, PI->height = h;
//         BBP_set_window_modes(PI);
//     }
// }
// 
// void BBP_reconfigure(plugin_info *PI)
// {
//     if (false == PI->inSlit)
//         GetMonitorRect(PI->hMon, &PI->mon_rect, GETMON_FROM_MONITOR);
// 
//     InvalidateRect(PI->hwnd, NULL, FALSE);
//     BBP_set_window_modes(PI);
// }

//===========================================================================
// bool BBP_get_rcpath(char *rcpath, HINSTANCE hInstance, const char *rcfile)
// {
//     return FindRCFile(rcpath, rcfile, hInstance);
// }

bool find_resource_file (wchar_t * pszOut, size_t sz, const wchar_t * filename, const wchar_t * basedir)
{
	wchar_t temp[MAX_PATH];

// 	if (defaultrc_path[0])
// 	{
// 		join_path(pszOut, defaultrc_path, file_basename(filename));
// 		if (FileExists(pszOut))
// 			return true;
// 	}
// #ifndef BBTINY
// 	sprintf(temp, "APPDATA\\blackbox\\%s", file_basename(filename));
// 	if (FileExists(replace_shellfolders(pszOut, temp, false)))
// 		return true;
// #endif
	if (bb::isAbsolutePath(filename))
	{
		_tcsncpy(pszOut, filename, sz);
		return bb::fileExists(pszOut);
	}

	if (basedir)
	{
		bb::joinPath(basedir, filename, pszOut, sz);
		// save as default for below
		_tcsncpy(temp, pszOut, sz);
		if (bb::fileExists(pszOut))
			return true;
	}

//	replace_shellfolders(pszOut, filename, false);

	if (bb::fileExists(pszOut))
		return true;

	if (basedir)
		_tcsncpy(pszOut, temp, sz);
	return false;
}

bool FindRCFile (wchar_t * buff, size_t buffsz, const wchar_t * filename, HINSTANCE module)
{
	wchar_t basedir[MAX_PATH];

	wchar_t dlldir_raw[MAX_PATH];
  if (NULL == module || 0 == GetModuleFileName(module, dlldir_raw, MAX_PATH))
	{
		return find_resource_file(buff, buffsz, filename, NULL);
  }
	else
	{
		wchar_t dlldir[MAX_PATH];
    GetLongPathName(dlldir_raw, dlldir, MAX_PATH);
    bb::getFileDirectory(dlldir, basedir, MAX_PATH);

		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];
		wchar_t fname[_MAX_FNAME];
		wchar_t fext[_MAX_EXT];
		bb::splitPath(filename, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT);
    if (_tcslen(fext) > 0)
		{
      // file has extension
      if (find_resource_file(buff, buffsz, filename, basedir))
				return true;
    }
		else
		{
			wchar_t try2[MAX_PATH];
			_snwprintf(try2, MAX_PATH, L"%s.rc", fname); // try file.rc
      if (find_resource_file(buff, buffsz, try2, basedir))
				return true;

			wchar_t try3[MAX_PATH];
			_snwprintf(try3, MAX_PATH, L"%src", fname); // try filerc
      if (find_resource_file(buff, buffsz, try3, basedir))
				return true;

      _tcsncpy(buff, try2, buffsz); // use file.rc as default
    }
  }
  return false;
}

bool BBP_get_rcpath (bbstring & rcpath, HINSTANCE hInstance, const wchar_t * rcfile)
{
	wchar_t p[MAX_PATH];
	bool const ret = FindRCFile(p, MAX_PATH, rcfile, hInstance);
	rcpath = std::move(bbstring(p));
	return ret;
}

bool BBP_get_rcpath (wchar_t * rcpath, size_t sz, HINSTANCE hInstance, const wchar_t * rcfile)
{
	return FindRCFile(rcpath, sz, rcfile, hInstance);
}

// bool BBP_get_rcpath(char *rcpath, HINSTANCE hInstance, const char *rcfile)
// {
//     return FindRCFile(rcpath, rcfile, hInstance);
// }


// void BBP_edit_file(const char *path)
// {
//     if (BBVERSION_LEAN) {
//         SendMessage(GetBBWnd(), BB_EDITFILE, (WPARAM)-1, (LPARAM)path);
//     } else {
//         char szTemp[MAX_PATH];
//         GetBlackboxEditor(szTemp);
//         BBExecute(NULL, NULL, szTemp, path, NULL, SW_SHOWNORMAL, false);
//     }
// }

//===========================================================================
#ifndef BBPLUGIN_NOMENU
//===========================================================================

bool BBP_broam_bool(struct plugin_info *PI, const char *temp, const char *key, bool *ip)
{
    int n = strlen(key);
    const char *s;
    if (_memicmp(temp, key, n))
        return false;
    s = temp + n;
    while (' ' == *s) ++s;
    if (0 == *s || 0 == _stricmp(s, "toggle"))
        *ip = false == *ip;
    else
    if (0 == _stricmp(s, "false"))
        *ip = false;
    else
    if (0 == _stricmp(s, "true"))
        *ip = true;
    else
        return false;
    if (PI) BBP_write_bool(PI, key, *ip);
    return true;
}

bool BBP_broam_int(struct plugin_info *PI, const char *temp, const char *key, int *ip)
{
    int n = strlen(key);
    const char *s;
    if (_memicmp(temp, key, n))
        return false;
    s = temp + n;
    if (' ' != *s)
        return false;
    while (' ' == *s)
        ++s;
    if (0 == *s)
        return false;
    *ip = atoi(s);
    if (PI) BBP_write_int(PI, key, *ip);
    return true;
}

bool BBP_broam_string(struct plugin_info *PI, const char *temp, const char *key, const char **ps)
{
    int n = strlen(key);
    const char *s;
    if (_memicmp(temp, key, n))
        return false;
    s = temp + n;
    if (' ' != *s)
        return false;
    while (' ' == *s)
        ++s;
    if (0 == *s)
        return false;
    *ps = s;
    if (PI) BBP_write_string(PI, key, s);
    return true;
}


bool BBP_broam_bool(struct plugin_info *PI, const wchar_t *temp, const wchar_t *key, bool *ip)
{
	int n = _tcslen(key);
	const wchar_t *s;
	if (_memicmp(temp, key, n))
		return false;
	s = temp + n;
	while (' ' == *s) ++s;
	if (0 == *s || 0 == _tcsicmp(s, L"toggle"))
		*ip = false == *ip;
	else
		if (0 == _tcsicmp(s, L"false"))
			*ip = false;
		else
			if (0 == _tcsicmp(s, L"true"))
				*ip = true;
			else
				return false;
	if (PI)
		BBP_write_bool(PI, key, *ip);
	return true;
}

bool BBP_broam_int(struct plugin_info *PI, const wchar_t *temp, const wchar_t *key, int *ip)
{
	int n = _tcslen(key);
	const wchar_t *s;
	if (_memicmp(temp, key, n))
		return false;
	s = temp + n;
	if (' ' != *s)
		return false;
	while (' ' == *s)
		++s;
	if (0 == *s)
		return false;
	*ip = _ttoi(s);
	if (PI)
		BBP_write_int(PI, key, *ip);
	return true;
}

bool BBP_broam_string(struct plugin_info *PI, const wchar_t *temp, const wchar_t *key, const wchar_t **ps)
{
	int n = _tcslen(key);
	const wchar_t *s;
	if (_memicmp(temp, key, n))
		return false;
	s = temp + n;
	if (' ' != *s)
		return false;
	while (' ' == *s)
		++s;
	if (0 == *s)
		return false;
	*ps = s;
	if (PI)
		BBP_write_string(PI, key, s);
	return true;
}



int BBP_handle_broam(struct plugin_info *PI, const char *temp)
{
    int v;
    const char *s;

    if (BBP_broam_bool(PI, temp, "useSlit", &PI->useSlit))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_bool(PI, temp, "pluginToggle", &PI->pluginToggle))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }
/*
    if (BBP_broam_bool(PI, temp, "visible", &PI->visible))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }
*/
    if (BBP_broam_bool(PI, temp, "alwaysOnTop", &PI->alwaysOnTop))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_bool(PI, temp, "clickRaise", &PI->clickRaise))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_bool(PI, temp, "AutoHide", &PI->autoHide))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_int(PI, temp, "snapWindow", &PI->snapWindow))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_bool(PI, temp, "alpha.enabled", &PI->alphaEnabled))
    {
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_int(PI, temp, "icon.saturation", &v))
    {
        //PI->saturation = (BYTE)eightScale_up(v);
        PI->saturation = (BYTE)v;
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_int(PI, temp, "icon.hue", &v))
    {
        //PI->hue = (BYTE)eightScale_up(v);
        PI->hue = (BYTE)v;
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_int(PI, temp, "alpha.value", &v))
    {
        //PI->alphaValue = (BYTE)eightScale_up(v);
        PI->alphaValue = (BYTE)v;
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_string(PI, temp, "placement", &s))
    {
//         int n = get_string_index(s, placement_strings);
//         if (-1 != n) {
//             BBP_set_place(PI, n);
//         }
        return BBP_BROAM_HANDLED;
    }

    if (!_stricmp(temp, "about"))
    {
        PI->about_box();
        return BBP_BROAM_HANDLED;
    }

    if (!_stricmp(temp, "editRC"))
    {
//        BBP_edit_file(PI->rcpath);
        return BBP_BROAM_HANDLED;
    }

    if (0 == _stricmp(temp, "readme"))
    {
//         char temp[MAX_PATH];
//         BBP_edit_file(set_my_path(PI->hInstance, temp, "readme.txt"));
        return BBP_BROAM_HANDLED;
    }

    if (!_stricmp(temp, "LoadDocs"))
    {
//         char docspath[MAX_PATH];
// 		locate_file(PI->hInstance, docspath, PI->class_name, "html");
//         BBExecute(NULL, "open", docspath, NULL, NULL, SW_SHOWNORMAL, false);
        return BBP_BROAM_HANDLED;
    }

    if (BBP_broam_string(PI, temp, "orientation", &s))
    {
        if (!_stricmp(s, "vertical"))
            PI->orient_vertical = true;
        else
        if (!_stricmp(s, "horizontal"))
            PI->orient_vertical = false;
        BBP_set_window_modes(PI);
        return BBP_BROAM_HANDLED | BBP_BROAM_METRICS;
    }

    return 0;
}

int BBP_handle_broam (plugin_info * PI, const wchar_t * temp)
{
	int v;
	const wchar_t *s;

	if (BBP_broam_bool(PI, temp, L"useSlit", &PI->useSlit))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_bool(PI, temp, L"pluginToggle", &PI->pluginToggle))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}
	/*
	if (BBP_broam_bool(PI, temp, L"visible", &PI->visible))
	{
	BBP_set_window_modes(PI);
	return BBP_BROAM_HANDLED;
	}
	*/
	if (BBP_broam_bool(PI, temp, L"alwaysOnTop", &PI->alwaysOnTop))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_bool(PI, temp, L"clickRaise", &PI->clickRaise))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_bool(PI, temp, L"AutoHide", &PI->autoHide))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_int(PI, temp, L"snapWindow", &PI->snapWindow))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_bool(PI, temp, L"alpha.enabled", &PI->alphaEnabled))
	{
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_int(PI, temp, L"icon.saturation", &v))
	{
		//PI->saturation = (BYTE)eightScale_up(v);
		PI->saturation = (BYTE)v;
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_int(PI, temp, L"icon.hue", &v))
	{
		//PI->hue = (BYTE)eightScale_up(v);
		PI->hue = (BYTE)v;
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_int(PI, temp, L"alpha.value", &v))
	{
		//PI->alphaValue = (BYTE)eightScale_up(v);
		PI->alphaValue = (BYTE)v;
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_string(PI, temp, L"placement", &s))
	{
		//         int n = get_string_index(s, placement_strings);
		//         if (-1 != n) {
		//             BBP_set_place(PI, n);
		//         }
		return BBP_BROAM_HANDLED;
	}

	if (!_tcsicmp(temp, L"about"))
	{
		PI->about_box();
		return BBP_BROAM_HANDLED;
	}

	if (!_tcsicmp(temp, L"editRC"))
	{
		//        BBP_edit_file(PI->rcpath);
		return BBP_BROAM_HANDLED;
	}

	if (0 == _tcsicmp(temp, L"readme"))
	{
		//         char temp[MAX_PATH];
		//         BBP_edit_file(set_my_path(PI->hInstance, temp, "readme.txt"));
		return BBP_BROAM_HANDLED;
	}

	if (!_tcsicmp(temp, L"LoadDocs"))
	{
		//         char docspath[MAX_PATH];
		// 		locate_file(PI->hInstance, docspath, PI->class_name, "html");
		//         BBExecute(NULL, "open", docspath, NULL, NULL, SW_SHOWNORMAL, false);
		return BBP_BROAM_HANDLED;
	}

	if (BBP_broam_string(PI, temp, L"orientation", &s))
	{
		if (!_tcsicmp(s, L"vertical"))
			PI->orient_vertical = true;
		else
			if (!_tcsicmp(s, L"horizontal"))
				PI->orient_vertical = false;
		BBP_set_window_modes(PI);
		return BBP_BROAM_HANDLED | BBP_BROAM_METRICS;
	}

	return 0;
}

//===========================================================================
#endif // def BBPLUGIN_NOMENU
//===========================================================================
// autohide

bool check_mouse(HWND hwnd)
{
    POINT pt;
    RECT rct;
    if (GetCapture() == hwnd)
        return 1;
    GetCursorPos(&pt);
    GetWindowRect(hwnd, &rct);
    if (PtInRect(&rct, pt))
        return 1;
    return 0;
}

void set_autohide_timer(plugin_info *PI, bool set)
{
    if (set)
        SetTimer(PI->hwnd, AUTOHIDE_TIMER, 100, NULL);
    else
        KillTimer(PI->hwnd, AUTOHIDE_TIMER);
}

void BBP_set_autoHide(plugin_info *PI, bool set)
{
    if (set != PI->autoHide)
    {
        PI->autoHide = set;
//        write_rc(PI, &PI->autoHide);
    }

    PI->suspend_autohide = false;
    PI->auto_shown = set && check_mouse(PI->hwnd);
    if (PI->auto_shown)
        set_autohide_timer(PI, true);
    BBP_set_window_modes(PI);
}

//===========================================================================

//===========================================================================
/*
    // handled messages

    WM_NCCREATE:
    BB_BROADCAST:

    WM_ENTERSIZEMOVE:
    WM_EXITSIZEMOVE:
    WM_WINDOWPOSCHANGING:

    BB_AUTORAISE:
    WM_MOUSEMOVE:
    WM_NCLBUTTONDOWN:
    WM_NCHITTEST:

    WM_TIMER:   // handle autohide
    WM_CLOSE:   // return 0;
*/

//===========================================================================

LRESULT CALLBACK BBP_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT msgs[] = { BB_RECONFIGURE, BB_BROADCAST, BB_DESKCLICK, 0};

    LRESULT Result = 0;
    plugin_info *PI  = (plugin_info *)GetWindowLongPtr(hwnd, 0);

    //dbg_printf("message %x", message);

    if (NULL == PI)
    {
        if (WM_NCCREATE == message)
        {
            // bind the window to the structure
            PI = (plugin_info *)((CREATESTRUCT*)lParam)->lpCreateParams;
            PI->hwnd = hwnd;
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)PI);
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    switch (message)
    {
        case WM_CREATE:
            SendMessage(GetBBWnd(), BB_REGISTERMESSAGE, (WPARAM)hwnd, (LPARAM)msgs);
            MakeSticky(hwnd);
            goto pass_nothing;

        case WM_DESTROY:
            SendMessage(GetBBWnd(), BB_UNREGISTERMESSAGE, (WPARAM)hwnd, (LPARAM)msgs);
            RemoveSticky(hwnd);
            goto pass_nothing;

        // ==========
        case BB_BROADCAST:
        {
            const wchar_t * temp = (LPWSTR)lParam;
            int f, len;

            if (0 == _tcsicmp(temp, L"@BBShowPlugins")) {
                PI->toggled_hidden = false;
                BBP_set_window_modes(PI);
                goto pass_result;
            }
            if (0 == _tcsicmp(temp, L"@BBHidePlugins")) {
                if (PI->pluginToggle) {
                    PI->toggled_hidden = true;
                    BBP_set_window_modes(PI);
                }
                goto pass_result;
            }

            if ('@' != *temp++)
                goto pass_nothing;

            len = PI->broam_key_len;
            if (len && 0 == _memicmp(temp, PI->broam_key, len) && '.' == temp[len]) {
                f = 0;
                temp += len + 1;
                goto do_broam;
            }

            if (PI->next)
                goto pass_nothing;

            len = PI->broam_key_len_common;
            if (len && 0 == _memicmp(temp, PI->broam_key, len)) {
                f = BBP_BROAM_COMMON;
                temp += len;
                goto do_broam;
            }

            goto pass_nothing;

        do_broam:
            f |= BBP_handle_broam(PI, temp);
            PI->process_broam(temp, f);
            goto pass_result;
        }

        // ==========

        case BB_DESKCLICK:
            if (lParam == 0
             && PI->clickRaise
             && false == PI->alwaysOnTop
             && false == PI->inSlit)
                SetWindowPos(hwnd, HWND_TOP,
                    0,0,0,0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
            goto pass_nothing;

        // ==========

        case WM_WINDOWPOSCHANGING:
            if (PI->is_moving) {
               if (false == PI->inSlit
                 && 0 == (0x8000 & GetAsyncKeyState(VK_SHIFT)))
                    SnapWindowToEdge((WINDOWPOS*)lParam, PI->snapWindow,
                        PI->is_sizing ? SNAP_FULLSCREEN|SNAP_SIZING
                        : SNAP_FULLSCREEN
                        );
                if (PI->is_sizing) {
                    WINDOWPOS* wp = (WINDOWPOS*)lParam;
                    if (wp->cx < 12) wp->cx = 12;
                    if (wp->cy < 12) wp->cy = 12;
                }
            }
            goto pass_nothing;

        case WM_WINDOWPOSCHANGED:
            if (PI->is_sizing) {
                WINDOWPOS* wp = (WINDOWPOS*)lParam;
                PI->width = wp->cx;
                PI->height = wp->cy;
                InvalidateRect(hwnd, NULL, FALSE);
            }
			SnapWindowToEdge((WINDOWPOS*)lParam, PI->snapWindow, true);
            goto pass_nothing;

        case WM_ENTERSIZEMOVE:
            PI->is_moving = true;
            goto pass_nothing;

        case WM_EXITSIZEMOVE:
            BBP_exit_moving(PI);
            BBP_set_autoHide(PI, PI->autoHide);
            if (PI->inSlit)
                SendMessage(PI->hSlit, SLIT_UPDATE, 0, (LPARAM)PI->hwnd);
            goto pass_nothing;

        // ==========
        case WM_LBUTTONDOWN:
            SetFocus(hwnd);
            UpdateWindow(hwnd);
            if (false == PI->inSlit && (MK_CONTROL & wParam)) {
                // start moving, when control-key is held down
                PostMessage(hwnd, WM_SYSCOMMAND, 0xf012, 0);
                goto pass_result;
            }
            goto pass_nothing;

        case WM_MOUSEMOVE:
            if (false == PI->mouse_over)
            {
                PI->mouse_over = true;
                set_autohide_timer(PI, true);
            }

            if (PI->auto_hidden)
            {
                PI->auto_shown = true;
                BBP_set_window_modes(PI);
                goto pass_result;
            }

            goto pass_nothing;

        case WM_TIMER:
            if (AUTOHIDE_TIMER != wParam)
                goto pass_nothing;

            if (check_mouse(hwnd))
                goto pass_result;
#if 0
            {
                POINT pt;
                GetCursorPos(&pt);
                if (PI->hMon != GetMonitorRect(&pt, NULL, GETMON_FROM_POINT))
                    goto pass_result;
            }
#endif
            if (PI->mouse_over) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);
                PostMessage(hwnd, WM_MOUSELEAVE, 0, MAKELPARAM(pt.x, pt.y));
                PI->mouse_over = false;
            }

            if (PI->auto_shown) {
                if (PI->suspend_autohide)
                    goto pass_result;
                PI->auto_shown = false;
                BBP_set_window_modes(PI);
            }

            set_autohide_timer(PI, false);
            goto pass_result;


        case BB_AUTOHIDE:
            if (PI->inSlit)
                PostMessage(PI->hSlit, message, wParam, lParam);

            if (wParam)
                PI->suspend_autohide |= lParam;
            else
                PI->suspend_autohide &= ~lParam;

            if (PI->suspend_autohide && PI->auto_hidden) {
                PI->auto_shown = true;
                BBP_set_window_modes(PI);
            }

            set_autohide_timer(PI, true);
            goto pass_result;

        case WM_CLOSE:
            goto pass_result;

        case WM_ERASEBKGND:
            Result = TRUE;
            goto pass_result;

        default:
        pass_nothing:
            return PI->wnd_proc(hwnd, message, wParam, lParam, NULL);
    }
pass_result:
    return PI->wnd_proc(hwnd, message, wParam, lParam, &Result);
}

//===========================================================================

//===========================================================================
struct class_info
{
    struct class_info * next;
    wchar_t name[48];
    HINSTANCE hInstance;
    int refc;
};


static struct class_info *CI;

static struct class_info **find_class(const wchar_t *name)
{
    struct class_info **pp;
    for (pp = &CI; *pp; pp = &(*pp)->next)
        if (0 == _tcsicmp((*pp)->name, name)) break;
    return pp;
}

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

static int class_info_register (const wchar_t * class_name, HINSTANCE hInstance)
{
    class_info * p = *find_class(class_name);
    if (NULL == p)
    {
        WNDCLASS wc;
        ZeroMemory(&wc,sizeof(wc));

        wc.lpfnWndProc  = BBP_WndProc;  // our window procedure
        wc.hInstance    = hInstance;    // hInstance of .dll
        wc.lpszClassName = class_name;  // our window class name
        wc.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;// | CS_DROPSHADOW;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.cbWndExtra = sizeof (void*);

        if (FALSE == RegisterClass(&wc))
        {
            //dbg_printf("failed to register %s", wc.lpszClassName);
            return 0;
        }

        p = (struct class_info*)m_alloc(sizeof(struct class_info));
        p->next = CI;
        CI = p;
        p->refc = 0;
        _tcscpy(p->name, class_name);
        p->hInstance = hInstance;
        //dbg_printf("registered class <%s> %x", wc.lpszClassName, wc.hInstance);
    }

    p->refc ++;
    return 1;
}

static void class_info_decref (const wchar_t * name)
{
    struct class_info *p, **pp = find_class(name);
    if (NULL != (p = *pp) && --p->refc <= 0)
    {
        UnregisterClass(p->name, p->hInstance);
        //dbg_printf("unregistered class <%s> %x", p->name, p->hInstance);
        *pp = p->next;
        m_free(p);
    }
}

int BBP_Init_Plugin (plugin_info * PI)
{
//     if (PI->broam_key)
// 		{
//         const char *s;
//         PI->broam_key_len = strlen(PI->broam_key);
//         s = strchr(PI->broam_key, '.');
//         if (s)
// 					PI->broam_key_len_common = s - PI->broam_key + 1;
//     }

    if (0 == class_info_register(PI->class_name, PI->hInstance))
        return 0;

    //dbg_printf("creating window <%s>", PI->class_name);
    PI->hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        PI->class_name,
        NULL,
        WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        PI->xpos,
        PI->ypos,
        PI->width,
        PI->height,
        NULL,           // parent window
        NULL,           // no menu
        PI->hInstance,  // hInstance of .dll
        PI              // init_data
        );

    if (NULL == PI->hwnd)
    {
        class_info_decref(PI->class_name);
        return 0;
    }

	// Transparency is only supported under Windows 2000/XP...
		PI->usingWin2kPlus = true; // GetOSVersion() >= 50;
    PI->is_alpha = *(BYTE *)GetSettingPtr(SN_MENUALPHA);
    PI->visible = true;

    BBP_set_window_modes(PI);
    return 1;
}

void BBP_Exit_Plugin(plugin_info *PI)
{
    //dbg_printf("window destroying <%s>", PI->class_name);
    if (PI->hwnd)
    {
        if (PI->inSlit)
            SendMessage(PI->hSlit, SLIT_REMOVE, 0, (LPARAM)PI->hwnd);
        DestroyWindow(PI->hwnd);
        class_info_decref(PI->class_name);
    }
}

static DWORD WINAPI QuitThread (void *pv)
{
    FreeLibraryAndExitThread((HMODULE)pv, 0);
#if _MSC_VER < 1400
    return 0; // never returns
#endif
}

int BBP_messagebox(
    plugin_info *PI,
    int flags,
    const char *fmt, ...)
{
//     va_list args;
//     char buffer[4000];
//     DWORD threadId;
//     HMODULE hLib;
// 
//     va_start(args, fmt);
//     GetModuleFileName(PI->hInstance, buffer, sizeof buffer);
//     hLib = LoadLibrary(buffer);
//     vsprintf(buffer, fmt, args);
//     flags = MessageBox(NULL, buffer, PI->class_name, flags|MB_TOPMOST|MB_SETFOREGROUND);
//     CloseHandle(CreateThread(NULL, 0, QuitThread, hLib, 0, &threadId));

    return flags;
}

//===========================================================================
