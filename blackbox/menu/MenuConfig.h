#pragma once
#include <vector>
#include <memory>
namespace YAML { class Node; }
namespace bb {

	enum MenuItemType {
		e_MenuItemSeparator,
		e_MenuItemFolder,
		e_MenuItemExec,
		e_MenuItemMenu,
	};

	struct MenuConfig;

	struct MenuItem
	{
		bbstring m_name;
		MenuItemType m_type;
		std::vector<bbstring> m_values;
		std::unique_ptr<MenuConfig> m_menu;
	};

	struct MenuConfig
	{
		bbstring m_name;
		std::vector<std::unique_ptr<MenuItem>> m_items;

		//void clear () { m_items.clear(); }
	};

	bool loadMenuConfig (YAML::Node & y_root, MenuConfig & config);
}


