/*===================================================

	MENU MASTER CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <blackbox/BlackBox_compat.h>

//Includes
#include "AgentMaster.h"
#include "ControlMaster.h"
#include "WindowMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "PluginMaster.h"
#include "ListMaster.h"
#include "MenuMaster.h"
#include "ModuleMaster.h"
//#include "../../BBPLugin/BBPlugin.h"

//Variables

//Internally defined functions
void menu_control_interfaceoptions(std::shared_ptr<bb::MenuConfig> m, control *c);
void menu_control_pluginoptions(std::shared_ptr<bb::MenuConfig> m);
void menu_control_controloptions(std::shared_ptr<bb::MenuConfig> m, control *c, wchar_t *title);
void menu_control_windowoptions(std::shared_ptr<bb::MenuConfig> m, control *c, wchar_t *title);

static bool menu_initialize(control *c, bool pop);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//menu_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int menu_startup()
{
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//menu_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int menu_shutdown()
{
	menu_initialize(NULL, true);
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//menu_control
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void menu_control(control *c, bool pop)
{
	// -----------------------------------
	//Variables
	wchar_t temp[120];
	std::shared_ptr<bb::MenuConfig> m;

	// menu update support
	if (false == menu_initialize(c, pop))
		return;

	//Make the new menu
	//NOTE: Include module name in menu title? Possible menu name duplication is already resolved in make_menu
	swprintf(temp, 120, L"%s (%s)", c->controlname,  c->controltypeptr->controltypename);
	m = make_menu(temp, c);

	//Add the appropriate menus
	menu_control_interfaceoptions(m, c);
	menu_control_controloptions(m, c, L"Control Options");
	menu_control_windowoptions(m, c, L"Window Options");
/*
	if (c->parentptr)
	{
		menu_control_controloptions(m, c->parentptr, "Parent Control Options");
		menu_control_windowoptions(m, c->parentptr, "Parent Window Options");
	}
*/
	make_submenu_item(m, L"Global Options", plugin_menu_settings());
	menu_control_pluginoptions(m);

	//Show the menu
	show_menu(c, m, pop);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//menu_control
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::shared_ptr<bb::MenuConfig>  menu_control_submenu(control *c)
{
	// NOTE: Currently unused, could be in the Edit Module > Control list.
	// -----------------------------------
	//Variables
	wchar_t temp[120]; std::shared_ptr<bb::MenuConfig> m;

	//Make the new menu
	swprintf(temp, 120, L"%s (%s)", c->controlname,  c->controltypeptr->controltypename);
	m = make_menu(temp, c);

	//Add the appropriate menus
	menu_control_interfaceoptions(m, c);
	menu_control_controloptions(m, c, L"Control Options");
	menu_control_windowoptions(m, c, L"Window Options");
	return m;
}


void menu_update_global(void)
{
	if (false == menu_initialize(NULL, false))
		return;

	show_menu(NULL, plugin_menu_settings(), false);
}

void menu_update_modules(void)
{
	if (false == menu_initialize(NULL, false))
		return;

	show_menu(NULL, module_menu_modulelist(), false);
}

void menu_update_editmodules(void)
{
	if (false == menu_initialize(NULL, false))
		return;

	show_menu(NULL, module_menu_editmodules(), false);
}

void menu_update_setactivemodule(void)
{
	if (false == menu_initialize(NULL, false))
		return;

	show_menu(NULL, module_menu_setactivemodule(), false);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//menu_controloptions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void menu_controloptions(std::shared_ptr<bb::MenuConfig> m, control *c, int count, agent *agents[], wchar_t *actionprefix, wchar_t *actions[], const int types[])
{
	//Declare variables
	wchar_t completeactionname[1024];

	for (int i = 0; i < count; i++)
	{
		//Make the complete action name
		swprintf(completeactionname, 1024, L"%s%s", actionprefix, actions[i]);

		//Make the submenu for this action
		std::shared_ptr<bb::MenuConfig> submenu = make_menu(actions[i], c);

		//If there is an agent, add its options menu
		if (agents[i])
		{
			make_menuitem_nop(submenu, agents[i]->agenttypeptr->agenttypenamefriendly);
			std::shared_ptr<bb::MenuConfig> submenu2 = make_menu(L"Options", c);
			agent_menu_context(submenu2, c, agents[i]);
			make_submenu_item(submenu, L"Options", submenu2);
			make_menuitem_nop(submenu, NULL);
			make_menuitem_nop(submenu, NULL);
		}


		//Make the agent selection menus for this action
		agent_menu_set(submenu, c, agents[i], types[i], completeactionname);

		//Add the submenu to the parent menu
		make_submenu_item(m, actions[i], submenu);
	}
}

//##################################################
//menu_control_interfaceoptions
//##################################################

void menu_control_interfaceoptions(std::shared_ptr<bb::MenuConfig> m, control *c)
{
	//Variables
	std::shared_ptr<bb::MenuConfig> submenu, submenu2;
	//Make the operations menu  
	submenu = make_menu(L"Interface Operations", c);

	//Create new controls
	submenu2 = make_menu(L"Create New Control", c);
	control_menu_create(submenu2, c, false);
	make_submenu_item(submenu, L"Create New Control", submenu2);
	if (c->controltypeptr->can_parent) //If it can be a parent
	{
		submenu2 = make_menu(L"Create New Child Control", c);
		control_menu_create(submenu2, c, true);
		make_submenu_item(submenu, L"Create New Child Control", submenu2);
	}
	make_menuitem_cmd(submenu,L"Create New Module",config_get_module_create());
	make_menuitem_nop(submenu, NULL);
	control_menu_settings(submenu, c);
	make_submenu_item(m, L"Interface Operations", submenu);
}

//##################################################
//menu_control_pluginoptions
//##################################################
void menu_control_pluginoptions(std::shared_ptr<bb::MenuConfig> m)
{
	std::shared_ptr<bb::MenuConfig> submenu;
	submenu = make_menu(L"Configuration");
	make_submenu_item(submenu, L"Modules", module_menu_modulelist());
	make_submenu_item(submenu, L"Edit Modules", module_menu_editmodules());
	make_submenu_item(submenu, L"Set Default Module", module_menu_setactivemodule());
	make_menuitem_nop(submenu, NULL);
	make_menuitem_cmd(submenu, L"Load Configuration Script...", config_get_plugin_load_dialog());
	make_menuitem_cmd(submenu, L"Edit Configuration Script", config_get_plugin_edit());
	make_menuitem_nop(submenu, NULL);
	make_menuitem_cmd(submenu, L"Save Configuration", config_get_plugin_save());
	make_menuitem_cmd(submenu, L"Save Configuration As...", config_get_plugin_saveas());
	make_menuitem_nop(submenu, NULL);
	make_menuitem_cmd(submenu, L"Reload Configuration", config_get_plugin_revert());
	make_submenu_item(m, L"Configuration", submenu);

	submenu = make_menu(L"Help");
	make_menuitem_cmd(submenu, L"Quick Reference...", L"@BBInterface Plugin About QuickHelp");
	make_menuitem_cmd(submenu, L"About...", L"@BBInterface Plugin About");
	make_menuitem_cmd(submenu, L"Show Welcome Interface", L"@BBInterface Plugin Load WelcomeScript\\welcome.rc");
	make_submenu_item(m, L"Help", submenu);
}


//##################################################
//controltype_button_startup
//##################################################
void menu_control_controloptions(std::shared_ptr<bb::MenuConfig> m, control *c, wchar_t *title)
{
	//Variables
	std::shared_ptr<bb::MenuConfig> submenu;

	//Make the operations menu  
	submenu = make_menu(title, c);

	//Make the name option
	make_menuitem_str(
		submenu,
		L"Control Name:",
		config_getfull_control_renamecontrol_s(c),
		c->controlname
		);
	make_menuitem_nop(submenu, L"");

	control_menu_context(submenu, c);
	make_submenu_item(m, title, submenu);
}

//##################################################
//controltype_button_startup
//##################################################
void menu_control_windowoptions(std::shared_ptr<bb::MenuConfig> m, control *c, wchar_t *title)
{
	//Variables
	std::shared_ptr<bb::MenuConfig> submenu;

	//Make the operations menu  
	submenu = make_menu(title, c);
	window_menu_context(submenu, c);
	make_submenu_item(m, title, submenu);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// blackbox menu wrapper
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// for convenience the unique menu id's are generated automatically from
// the control-name and a incremented number.

#define COMMON_MENU

#if 1
static int menu_num_global;
static bool menu_pop_global;
static std::shared_ptr<bb::MenuConfig>  last_menu_global;

static wchar_t *get_menu_id(wchar_t *buffer, size_t n, control *c, module* m, const wchar_t *title)
{
	if (c)
	{
		if (title)
			swprintf(buffer, n, L"BBIMenu::Control-%s-%s-%s-%d", c->moduleptr->name, c->controlname, title, ++menu_num_global);
		else
			swprintf(buffer, n, L"BBIMenu::Control-%s-%s", c->moduleptr->name, c->controlname);
	}
	else if (m)
	{
		swprintf(buffer, n, L"BBIMenu::Module-%s-%s", m->name, title ? title : L"");
	}
	else
	{
		swprintf(buffer, n, L"BBIMenu::Global-%s", title ? title : L"");
	}
	return buffer;
}

static bool defMenuExists(LPCWSTR IDString_start)
{
	return true;
}

static bool menu_exists(control *c)
{
	static bool (*pMenuExists)(LPCWSTR IDString_start);
	if (NULL == pMenuExists)
	{
		*(FARPROC*)&pMenuExists = GetProcAddress(GetModuleHandle(NULL), L"MenuExists");
		if (NULL == pMenuExists) pMenuExists = defMenuExists;
	}

	wchar_t menu_id[200];
	get_menu_id(menu_id, 200, c, NULL, NULL);
	return pMenuExists(menu_id);
}

static bool menu_initialize(control *c, bool pop)
{
	menu_pop_global = pop;
	menu_num_global = 0;
#ifdef COMMON_MENU
	return pop || menu_exists(c);
#else
	if (BBVERSION_LEAN == BBVersion)
		return pop || menu_exists(c);

	if (pop && BBVERSION_09X == BBVersion && last_menu_global)
		DelMenu(last_menu_global), last_menu_global = NULL;

	return pop;
#endif
}

std::shared_ptr<bb::MenuConfig> make_menu(const wchar_t *title, control* c)
{
#ifndef COMMON_MENU
	if (BBVERSION_LEAN != BBVersion)
		return MakeMenu(title);
#endif
	wchar_t menu_id[356];
	get_menu_id(menu_id, 356, c, NULL, title);
	return MakeNamedMenu(title, menu_id, menu_pop_global);
}

std::shared_ptr<bb::MenuConfig> make_menu(const wchar_t *title, module* m)
{
#ifndef COMMON_MENU
	if (BBVERSION_LEAN != BBVersion)
		return MakeMenu(title);
#endif
	wchar_t menu_id[300];
	get_menu_id(menu_id, 300, NULL, m, title);
	return MakeNamedMenu(title, menu_id, menu_pop_global);
}


void make_menuitem_str(std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd, const wchar_t * init_string)
{
	wchar_t buffer[BBI_MAX_LINE_LENGTH];

//	if (BBP_is_bbversion_lean())
	swprintf(buffer, BBI_MAX_LINE_LENGTH,L"%s \"%%s\"", cmd), cmd = buffer;

// 	if (BBP_is_bbversion_09x())
// 		swprintf(buffer, BBI_MAX_LINE_LENGTH, L"\"%s\"", init_string), init_string = buffer;

	MakeMenuItemString(m, title, cmd, init_string);
}

void make_menuitem_int(std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd, int val, int minval, int maxval)
{
// 	if (BBP_is_bbversion_09x())
// 	{
// 		wchar_t buffer[20];
// 		swprintf(buffer, 20, L"%d", val);
// 		MakeMenuItemString(m, title, cmd, buffer);
// 	}
// 	else
// 	{
		MakeMenuItemInt(m, title, cmd, val, minval, maxval);
//	}
}

void make_menuitem_bol(std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd, bool checked)
{
	MakeMenuItemBool(m, title, cmd, checked);
}

void make_menuitem_cmd(std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, const wchar_t* cmd)
{
	MakeMenuItem(m, title, cmd);
}

void make_menuitem_nop(std::shared_ptr<bb::MenuConfig> m, const wchar_t *title)
{
	MakeMenuNOP(m, title);
}

void make_submenu_item(std::shared_ptr<bb::MenuConfig> m, const wchar_t *title, std::shared_ptr<bb::MenuConfig> sub)
{
	MakeSubmenu(m, sub, title);
}

void show_menu(control *c, std::shared_ptr<bb::MenuConfig> m, bool pop)
{
	last_menu_global = m;
	ShowMenu(m);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#else
#define new_str new_string
#define free_str free_string
#include "NewMenu.cpp"
#endif
