#pragma once

struct module; class Menu; struct agent; struct control;

int menu_startup ();
int menu_shutdown ();

void menu_control (control *c, bool pop);
void menu_update_global ();
void menu_update_modules ();
void menu_update_editmodules ();
void menu_update_setactivemodule ();
void menu_controloptions (Menu *m, control *c, int count, agent *agents[], wchar_t *actionprefix, wchar_t *actions[], const int types[]);
Menu* menu_control_submenu (control *c);

Menu * make_menu (const wchar_t *title, control *c = NULL);
Menu * make_menu (const wchar_t *title, module *m);
// Use a single argument for global menus, provide an addition control/module pointer, if it makes sense
void make_menuitem_str (Menu *m, const wchar_t *title, const wchar_t* cmd, const wchar_t * init_string);
void make_menuitem_int (Menu *m, const wchar_t *title, const wchar_t* cmd, int val, int minval, int maxval);
void make_menuitem_bol (Menu *m, const wchar_t *title, const wchar_t* cmd, bool checked);
void make_menuitem_cmd (Menu *m, const wchar_t *title, const wchar_t* cmd);
void make_menuitem_nop (Menu *m, const wchar_t *title);
void make_submenu_item (Menu *m, const wchar_t *title, Menu *sub);
void show_menu (control *c, Menu *m, bool pop);

