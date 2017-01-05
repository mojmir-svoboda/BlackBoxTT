#pragma once
#include <vector>
#include <memory>
#include <bblib/bbstring.h>
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

	struct MenuItem
	{
		bbstring m_name;
		MenuItemType m_type { e_MenuItemSeparator };
		bbstring m_value;
		std::unique_ptr<MenuConfig> m_menu;

		MenuItem () { }
		MenuItem (MenuItem const & rhs);
		MenuItem & operator= (MenuItem const & rhs);
	};

	struct MenuConfig
	{
		bbstring m_name;
		std::vector<MenuItem> m_items;

		//void clear () { m_items.clear(); }
	};

	inline MenuItem::MenuItem (MenuItem const & rhs)
		: m_name(rhs.m_name)
		, m_type(rhs.m_type)
		, m_value(rhs.m_value)
		, m_menu(rhs.m_menu ? new MenuConfig(*rhs.m_menu) : nullptr)
	{ }

	inline MenuItem & MenuItem::operator= (MenuItem const & rhs)
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


