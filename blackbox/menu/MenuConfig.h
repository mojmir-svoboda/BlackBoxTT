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
		e_MenuItemMenu,
		e_MenuItemScript,
	};

	struct MenuConfig;

	struct MenuConfigItem
	{
		bbstring m_name;
		MenuItemType m_type { e_MenuItemSeparator };
		bbstring m_value;
		std::unique_ptr<MenuConfig> m_menu;

		MenuConfigItem () { }
		MenuConfigItem (MenuConfigItem const & rhs);
		MenuConfigItem & operator= (MenuConfigItem const & rhs);
	};

	struct MenuConfig : WidgetConfig
	{
		std::vector<MenuConfigItem> m_items;

		//void clear () { m_items.clear(); }
	};

	inline MenuConfigItem::MenuConfigItem (MenuConfigItem const & rhs)
		: m_name(rhs.m_name)
		, m_type(rhs.m_type)
		, m_value(rhs.m_value)
		, m_menu(rhs.m_menu ? new MenuConfig(*rhs.m_menu) : nullptr)
	{ }

	inline MenuConfigItem & MenuConfigItem::operator= (MenuConfigItem const & rhs)
	{
		if (this != &rhs)
		{
			m_name = rhs.m_name;
			m_type = rhs.m_type;
			m_value = rhs.m_value;
			if (rhs.m_menu)
			{
				std::unique_ptr<MenuConfig> cpy(new MenuConfig(*rhs.m_menu));
				m_menu = std::move(cpy);
			}
		}
		return *this;
	}
	

	bool loadMenuConfig (YAML::Node & y_root, MenuConfig & config);
}


