#pragma once
#include <vector>
#include <memory>
#include <bblib/bbstring.h>
#include <blackbox/WidgetConfig.h>
namespace YAML { class Node; }
namespace bb {

	enum MenuItemType : uint32_t {
		e_MenuItemSeparator,
 		e_MenuItemFolder,
		e_MenuItemSubMenuFolder,
// 		e_MenuItemExec,
		e_MenuItemSubMenu,
		e_MenuItemScript,
		e_MenuItemInt,
		e_MenuItemCheckBox,
		e_MenuItemBroam,
		e_MenuItemBroamBool,
		e_MenuItemBroamInt,
		e_MenuItemBroamString,
	};

	struct MenuConfig;

	struct MenuConfigItem
	{
		MenuItemType m_type { e_MenuItemSeparator };
		bbstring m_name;

		MenuConfigItem () { }
		MenuConfigItem (MenuItemType type) : m_type(type) { }
		MenuConfigItem (MenuItemType type, bbstring const & name) : m_type(type), m_name(name) { }
	};

	struct MenuConfigItemSeparator : MenuConfigItem
	{
		MenuConfigItemSeparator () : MenuConfigItem(e_MenuItemSeparator) { }
		MenuConfigItemSeparator (bbstring const & name) : MenuConfigItem(e_MenuItemSeparator, name) { }
	};

	struct MenuConfigItemScript : MenuConfigItem
	{
		bbstring m_script;
		MenuConfigItemScript (bbstring const & name, bbstring const & script) : MenuConfigItem(e_MenuItemScript, name), m_script(script) { }
	};

	struct MenuConfigItemInt : MenuConfigItem
	{
		int m_min { 0 };
		int m_val { 0 };
		int m_max { 0 };
//		bbstring m_cmd;

		MenuConfigItemInt () : MenuConfigItem(e_MenuItemInt) { }
		MenuConfigItemInt (bbstring const & name/*, bbstring const & cmd*/, int min, int val, int max) : MenuConfigItem(e_MenuItemInt, name), m_min(min), m_val(val), m_max(max) { }
	};

	struct MenuConfigItemSubMenu : MenuConfigItem
	{
		std::shared_ptr<MenuConfig> m_menu;

		MenuConfigItemSubMenu (bbstring const & name, std::shared_ptr<MenuConfig> menu) : MenuConfigItem(e_MenuItemSubMenu, name), m_menu(menu) { }
		MenuConfigItemSubMenu (MenuItemType type, bbstring const & name, std::shared_ptr<MenuConfig> menu) : MenuConfigItem(type, name), m_menu(menu) { }
		MenuConfigItemSubMenu (MenuItemType type, bbstring const & name) : MenuConfigItem(type, name) { }
		MenuConfigItemSubMenu (MenuItemType type) : MenuConfigItem(type) { }
	};

	struct MenuConfigItemFolder : MenuConfigItem
	{
		bbstring m_folder;
		MenuConfigItemFolder (bbstring const & name, bbstring const & s) : MenuConfigItem(e_MenuItemFolder, name), m_folder(s) { }
		MenuConfigItemFolder () : MenuConfigItem(e_MenuItemFolder) { }
	};

	struct MenuConfigItemSubMenuFolder : MenuConfigItemSubMenu
	{
		bbstring m_folder;
		MenuConfigItemSubMenuFolder (bbstring const & name, bbstring const & s) : MenuConfigItemSubMenu(e_MenuItemSubMenuFolder, name), m_folder(s) { }
		MenuConfigItemSubMenuFolder () : MenuConfigItemSubMenu(e_MenuItemSubMenuFolder) { }
	};

	struct MenuConfigItemCheckBox : MenuConfigItem
	{
		bbstring m_getScript;
		bbstring m_onCheckScript;
		bbstring m_onUncheckScript;

		MenuConfigItemCheckBox () : MenuConfigItem(e_MenuItemCheckBox) { }
		MenuConfigItemCheckBox (bbstring const & name, bbstring const & get, bbstring const & chk, bbstring const & uchk) : MenuConfigItem(e_MenuItemCheckBox, name), m_getScript(get), m_onCheckScript(chk), m_onUncheckScript(uchk) { }
	};

	struct MenuConfig : WidgetConfig
	{
		std::vector<std::shared_ptr<MenuConfigItem>> m_items;

		void clear () { m_items.clear(); }
	};

	bool loadMenuConfig (YAML::Node & y_root, MenuConfig & config);


	// compatibility items
	struct MenuConfigItemBroam : MenuConfigItem
	{
		bbstring m_broam;
		MenuConfigItemBroam (bbstring const & name, bbstring const & broam) : MenuConfigItem(e_MenuItemBroam, name), m_broam(broam) { }
	};
	struct MenuConfigItemBroamBool : MenuConfigItem
	{
		bbstring m_broam;
		bool m_checked { false };
		MenuConfigItemBroamBool (bbstring const & name, bbstring const & broam, bool chk) : MenuConfigItem(e_MenuItemBroamBool, name), m_broam(broam), m_checked(chk) { }
	};
	struct MenuConfigItemBroamInt : MenuConfigItem
	{
		bbstring m_broam;
		int m_min { 0 };
		int m_val { 0 };
		int m_max { 0 };
		MenuConfigItemBroamInt (bbstring const & name, bbstring const & broam, int min, int val, int max) : MenuConfigItem(e_MenuItemBroamInt, name), m_broam(broam), m_min(min), m_val(val), m_max(max) { }
	};
	struct MenuConfigItemBroamString : MenuConfigItem
	{
		bbstring m_broam;
		bbstring m_text;
		MenuConfigItemBroamString (bbstring const & name, bbstring const & broam, bbstring const & text) : MenuConfigItem(e_MenuItemBroamString, name), m_broam(broam), m_text(text) { }
	};
}


