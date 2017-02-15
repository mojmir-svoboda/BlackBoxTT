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
		e_MenuItemExec,
		e_MenuItemSubMenu,
		e_MenuItemScript,
		e_MenuItemCheckBox,
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

	struct MenuConfigItemScript : MenuConfigItem
	{
		bbstring m_script;
		MenuConfigItemScript (bbstring const & name, bbstring const & script) : MenuConfigItem(e_MenuItemScript, name), m_script(script) { }
	};

	struct MenuConfigItemSubMenu : MenuConfigItem
	{
		std::shared_ptr<MenuConfig> m_menu;

		MenuConfigItemSubMenu (bbstring const & name, std::shared_ptr<MenuConfig> menu) : MenuConfigItem(e_MenuItemSubMenu, name), m_menu(menu) { }
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
}


