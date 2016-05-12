/*
 ============================================================================

  This file is part of the bbLean source code.

  Copyright © 2004-2009 grischka
  http://bb4win.sf.net/bblean
  Copyright © 2008-2009 The Blackbox for Windows Development Team

  http://developer.berlios.de/projects/bblean
  http://bb4win.sourceforge.net

  bb4win_mod and bbLean are free software, released under the GNU General Public License
  (GPL version 2).

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

 ============================================================================
*/
#pragma once
#include <platform_win.h>
#include "bblibcompat_api.h"
#include <bbstring.h>

struct plugin_info
{
    plugin_info * next;
    const wchar_t * class_name;
    HINSTANCE hInstance;
    HWND hwnd;

    HWND hSlit;
    HMONITOR hMon;
    RECT mon_rect;

    // config vars
    int place;

    int xpos;
    int ypos;
    int width;
    int height;
    int snapWindow;

    bool useSlit;
    bool alwaysOnTop;
    bool autoHide;
    bool usingWin2kPlus;
    bool alphaEnabled;
    BYTE alphaValue;

    int saturation;
    int hue;

    bool clickRaise;
    bool pluginToggle;
    bool visible;

    bool orient_vertical;
    bool is_bar;
    bool no_icons;

    // state vars
    bool inSlit;
    bool toggled_hidden;
    bool auto_hidden;
    bool auto_shown;
    bool mouse_over;
    char suspend_autohide;

    bool is_moving;
    bool is_sizing;
    bool is_visible;
    BYTE is_alpha;

    // misc
    wchar_t rcpath[MAX_PATH];
    wchar_t rc_key[256];
    wchar_t broam_key[256];
    int broam_key_len;
    int broam_key_len_common;

    plugin_info ()
			: next(nullptr)
			, class_name(nullptr)
			, hInstance(nullptr)
			, hwnd(nullptr)
			, hSlit(nullptr)
			, hMon(nullptr)
			, mon_rect()
			, place(0)
			, xpos(0)
			, ypos(0)
			, width(0)
			, height(0)
			, snapWindow(0)
			, useSlit(false)
			, alwaysOnTop(false)
			, autoHide(false)
			, usingWin2kPlus(false)
			, alphaEnabled(false)
			, alphaValue(0)
			, saturation(0)
			, hue(0)
			, clickRaise(false)
			, pluginToggle(false)
			, visible(false)
			, orient_vertical(false)
			, is_bar(false)
			, no_icons(false)
			, inSlit(false)
			, toggled_hidden(false)
			, auto_hidden(false)
			, auto_shown(false)
			, mouse_over(false)
			, suspend_autohide(0)
			, is_moving(false)
			, is_sizing(false)
			, is_visible(false)
			, is_alpha(0)
			, rcpath{0}
			, rc_key{0}
			, broam_key{0}
			, broam_key_len(0)
			, broam_key_len_common(0)
		{
		}
    virtual ~plugin_info() { }

    virtual void process_broam (const char * temp, int f) { }
		virtual void process_broam (const wchar_t * temp, int f) { }
    virtual void pos_changed () { }
    virtual void about_box () { }
    virtual LRESULT wnd_proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *ret) = 0;
};

enum Plugin_Positions
{
    POS_User        = 0,

    POS_TopLeft       ,
    POS_TopCenter     ,
    POS_TopRight      ,

    POS_BottomLeft    ,
    POS_BottomCenter  ,
    POS_BottomRight   ,

    POS_CenterLeft    ,
    POS_CenterRight   ,
    POS_Center        ,

    POS_Top           ,
    POS_Bottom        ,
    POS_Left          ,
    POS_Right         ,
    POS_CenterH ,
    POS_CenterV ,

    POS_LAST
};

//===========================================================================
// #ifdef __cplusplus
// extern "C" {
// #endif

// #ifdef BBP_LIB
// #define BBP_DLL_EXPORT __declspec(dllexport)
// #else
// #define BBP_DLL_EXPORT
// #endif

// #define BBVERSION_LEAN (BBP_bbversion()>=2)
// #define BBVERSION_XOB (BBP_bbversion()==1)
// #define BBVERSION_09X (BBP_bbversion()==0)

#define AUTOHIDE_TIMER 1

int  BBP_Init_Plugin(plugin_info * PI);
void BBP_Exit_Plugin(plugin_info *PI);
bool BBP_handle_message(plugin_info *PI, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
int BBP_messagebox(plugin_info *PI, int flags, const char *fmt, ...);

//bool BBP_get_rcpath(char *rcpath, HINSTANCE hInstance, const char *rcfile);
bool BBP_get_rcpath (wchar_t *rcpath, size_t buffsz, HINSTANCE hInstance, const wchar_t *rcfile);
bool BBP_get_rcpath (bbstring & rcpath, HINSTANCE hInstance, const wchar_t *rcfile);
bool BBP_read_window_modes(plugin_info *PI, const char *rcfile);
bool BBP_read_window_modes(plugin_info *PI, const wchar_t *rcfile);
void BBP_write_window_modes(plugin_info *PI);

void BBP_set_window_modes(plugin_info *PI);
void BBP_set_place(plugin_info *PI, int place);
void BBP_set_size(plugin_info *PI, int w, int h);
void BBP_set_visible(plugin_info *PI, bool hidden);
void BBP_set_autoHide(plugin_info *PI, bool set);

void BBP_reconfigure(plugin_info *PI);

const wchar_t * BBP_placement_string(int pos);
int BBP_get_placement(const wchar_t * place_string);

void BBP_write_string(plugin_info *PI, const char *rcs, const char *val);
void BBP_write_int(plugin_info *PI, const char *rcs, int val);
void BBP_write_bool(plugin_info *PI, const char *rcs, bool val);
void BBP_write_string(plugin_info *PI, const wchar_t *rcs, const wchar_t *val);
void BBP_write_int(plugin_info *PI, const wchar_t *rcs, int val);
void BBP_write_bool(plugin_info *PI, const wchar_t *rcs, bool val);


const char* BBP_read_string(plugin_info *PI, char *dest, const char *rcs, const char *def);
int  BBP_read_int(plugin_info *PI, const char *rcs, int def);
bool BBP_read_bool(plugin_info *PI, const char *rcs, bool def);
void BBP_rename_setting(plugin_info *PI, const char *rcs, const char *rcnew);
const char * BBP_read_value(plugin_info *PI, const char *rcs, LONG *pPos);

bool BBP_read_string(plugin_info *PI, wchar_t *dest, size_t const dest_sz, const wchar_t *rcs, const wchar_t *def);
int  BBP_read_int(plugin_info *PI, const wchar_t *rcs, int def);
bool BBP_read_bool(plugin_info *PI, const wchar_t *rcs, bool def);
void BBP_rename_setting(plugin_info *PI, const wchar_t *rcs, const wchar_t *rcnew);
const wchar_t * BBP_read_value(plugin_info *PI, const wchar_t *rcs, LONG *pPos);

void BBP_edit_file(const char *path);
int BBP_bbversion(void);

//int GetOSVersion(void);

//===========================================================================
typedef struct n_menu n_menu;

n_menu *n_makemenu(const char *title);
n_menu *n_submenu(n_menu *m, const char *text);

// Menu *n_convertmenu(n_menu *m, const char *broam_key, bool popup);
// void n_menuitem_nop(n_menu *m, const char *text ISNULL);
// void n_menuitem_cmd(n_menu *m, const char *text, const char *cmd);
// void n_menuitem_bol(n_menu *m, const char *text, const char *cmd, bool check);
// void n_menuitem_int(n_menu *m, const char *text, const char *cmd, int val, int vmin, int vmax);
// void n_menuitem_str(n_menu *m, const char *text, const char *cmd, const char *init);
// void n_disable_lastitem(n_menu *m);
// 
// void BBP_n_insertmenu(plugin_info *PI, n_menu *m);
// n_menu *BBP_n_placementmenu(plugin_info *PI, n_menu *m);
// n_menu *BBP_n_orientmenu(plugin_info *PI, n_menu *m);
// n_menu *BBP_n_windowmenu(plugin_info *PI, n_menu *m);
bool BBP_broam_int(plugin_info *PI, const wchar_t *temp, const char *key, int *ip);
bool BBP_broam_string(plugin_info *PI, const wchar_t *temp, const char *key, const char **ps);
bool BBP_broam_bool(plugin_info *PI, const wchar_t *temp, const char *key, bool *ip);
bool BBP_broam_int(plugin_info *PI, const wchar_t *temp, const wchar_t *key, int *ip);
bool BBP_broam_string(plugin_info *PI, const wchar_t *temp, const wchar_t *key, const wchar_t **ps);
bool BBP_broam_string(plugin_info *PI, const wchar_t *temp, const wchar_t *key, bbstring & val);
bool BBP_broam_bool(plugin_info *PI, const wchar_t *temp, const wchar_t *key, bool *ip);
// void n_showmenu(plugin_info *PI, n_menu *m, bool popup, int flags, ...);
// void BBP_showmenu(plugin_info *PI, Menu *pMenu, int flags);

//===========================================================================

bool check_mouse(HWND hwnd);

#ifndef __cplusplus
BBP_DLL_EXPORT plugin_info * BBP_create_info(void);
#endif

#define BBP_BROAM_HANDLED 1
#define BBP_BROAM_COMMON 4
#define BBP_BROAM_METRICS 2
// 
// #ifdef __cplusplus
// }
// #endif
//===========================================================================

