#pragma once
#include "common.h"
#include "blackbox_api.h"
#include "gfx/MenuWidget.h"
/* ------------------------------------ */
/* Plugin bb::MenuWidget API - See the SDK for application examples */

/* creates a Menu or Submenu, Id must be unique, fshow indicates whether
the menu should be shown (true) or redrawn (false) */
BB_API std::shared_ptr<bb::MenuConfig> MakeNamedMenu (const wchar_t * HeaderText, const wchar_t * Id, bool fshow);
BB_API bb::MenuConfigItem * MakeMenuGrip (std::shared_ptr<bb::MenuConfig> PluginMenu, const wchar_t * Title);

/* inserts an item to execute a command or to set a boolean value */
BB_API bb::MenuConfigItem * MakeMenuItem (std::shared_ptr<bb::MenuConfig> PluginMenu, const wchar_t * Title, const wchar_t * Cmd);
/* inserts an item to execute a command or to set a boolean value */
BB_API bb::MenuConfigItem * MakeMenuItemBool (std::shared_ptr<bb::MenuConfig> PluginMenu, const wchar_t * Title, const wchar_t * Cmd, bool ShowIndicator);

/* inserts an inactive item, optionally with text. 'Title' may be NULL. */
BB_API bb::MenuConfigItem * MakeMenuNOP (std::shared_ptr<bb::MenuConfig> PluginMenu, const wchar_t * Title);

/* inserts an item to adjust a numeric value */
BB_API bb::MenuConfigItem * MakeMenuItemInt (std::shared_ptr<bb::MenuConfig> PluginMenu, const wchar_t * Title, const wchar_t * Cmd, int val, int minval, int maxval);

/* inserts an item to edit a string value */
BB_API bb::MenuConfigItem * MakeMenuItemString (std::shared_ptr<bb::MenuConfig> PluginMenu, const wchar_t * Title, const wchar_t * Cmd, const wchar_t * init_string);

/* inserts an item, which opens a submenu */
BB_API bb::MenuConfigItem * MakeSubmenu (std::shared_ptr<bb::MenuConfig> ParentMenu, std::shared_ptr<bb::MenuConfig> ChildMenu, const wchar_t * Title);

/* inserts an item, which opens a submenu from a system folder.
'Cmd' optionally may be a Broam which then is sent on user click
with "%s" in the broam string replaced by the selected filename */
BB_API bb::MenuConfigItem * MakeMenuItemPath (std::shared_ptr<bb::MenuConfig> ParentMenu, const wchar_t * Title, const wchar_t * path, const wchar_t * Cmd);

/* Context menu for filesystem items. One of path or pidl can be NULL */
BB_API bb::MenuConfig * MakeContextMenu (const wchar_t * path, const void * pidl);

/* shows the menu */
BB_API void ShowMenu (std::shared_ptr<bb::MenuConfig> PluginMenu);

/* checks whether a menu with ID starting with 'IDString_start', still exists */
BB_API bool MenuExists (const wchar_t * IDString_start);

/* set option for bb::MenuConfigItem  */
BB_API void MenuItemOption (bb::MenuConfigItem * pItem, int option, ...);
#define BBMENUITEM_DISABLED   1 /* set disabled state */
#define BBMENUITEM_CHECKED    2 /* set checked state */
#define BBMENUITEM_LCOMMAND   3 /* next arg is command for left click */
#define BBMENUITEM_RCOMMAND   4 /* next arg is command for right click */
#define BBMENUITEM_OFFVAL     5 /* next args are offval, offstring (with Int-Items) */
#define BBMENUITEM_UPDCHECK   6 /* update checkmarks on the fly */
#define BBMENUITEM_JUSTIFY    7 /* next arg is DT_LEFT etc... */
#define BBMENUITEM_SETICON    8 /* next arg is "path\to\icon[,iconindex]" */
#define BBMENUITEM_SETHICON   9 /* next arg is HICON */
#define BBMENUITEM_RMENU     10 /* next arg is bb::MenuConfig* for right-click menu */

BB_API void MenuOption (std::shared_ptr<bb::MenuConfig> pMenu, int flags, ...);
#define BBMENU_XY             0x0001 /* next arg is x/y position */
#define BBMENU_RECT           0x0002 /* next arg is *pRect to show above/below */
#define BBMENU_CENTER         0x0003 /* center menu on screen */
#define BBMENU_CORNER         0x0004 /* align with corner on mouse */
#define BBMENU_POSMASK        0x0007 /* bit mask for above positions */
#define BBMENU_KBD            0x0008 /* use position from blackbox.rc */
#define BBMENU_XRIGHT         0x0010 /* x is menu's right */
#define BBMENU_YBOTTOM        0x0020 /* y is menu's bottom */
#define BBMENU_PINNED         0x0040 /* show menu initially pinned */
#define BBMENU_ONTOP          0x0080 /* show menu initially on top */
#define BBMENU_NOFOCUS        0x0100 /* dont set focus on menu */
#define BBMENU_NOTITLE        0x0200 /* no title */
#define BBMENU_MAXWIDTH       0x0400 /* next arg is maximal menu width */
#define BBMENU_SORT           0x0800 /* sort menu alphabetically */
#define BBMENU_ISDROPTARGET   0x1000 /* register as droptarget */
#define BBMENU_HWND           0x2000 /* next arg is HWND to send notification on menu-close */
#define BBMENU_SYSMENU        0x4000 /* is a system menu (for bbLeanSkin/Bar) */

/* obsolete: */
BB_API std::shared_ptr<bb::MenuConfig> MakeMenu (const wchar_t * HeaderText);
BB_API void DelMenu (std::shared_ptr<bb::MenuConfig> PluginMenu); /* does nothing */