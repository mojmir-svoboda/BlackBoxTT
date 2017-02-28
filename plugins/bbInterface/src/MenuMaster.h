#pragma once
#include <memory>
#include <blackbox/BlackBox_compat.h>

struct module; class Menu; struct agent; struct control;

int menu_startup ();
int menu_shutdown ();

void menu_control (control *c, bool pop);
void menu_update_global ();
void menu_update_modules ();
void menu_update_editmodules ();
void menu_update_setactivemodule ();
void menu_controloptions (std::shared_ptr<bb::MenuConfig> m, control *c, int count, agent *agents[], wchar_t *actionprefix, wchar_t *actions[], const int types[]);
std::shared_ptr<bb::MenuConfig>  menu_control_submenu (control *c);

std::shared_ptr<bb::MenuConfig> make_menu (const wchar_t *title, control *c = NULL);
std::shared_ptr<bb::MenuConfig> make_menu (const wchar_t *title, module *m);
// Use a single argument for global menus, provide an addition control/module pointer, if it makes sense
void make_menuitem_str (std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd, const wchar_t * init_string);
void make_menuitem_int (std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd, int val, int minval, int maxval);
void make_menuitem_bol (std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd, bool checked);
void make_menuitem_cmd (std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd);
void make_menuitem_nop (std::shared_ptr<bb::MenuConfig> m, const wchar_t *title);
void make_submenu_item (std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, std::shared_ptr<bb::MenuConfig> sub);
void show_menu (control *c, std::shared_ptr<bb::MenuConfig> m, bool pop);

