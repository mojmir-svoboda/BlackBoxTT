#include "MenuConfig.h"
#include "utils_yaml.h"
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<bb::MenuConfig>
	{
		static Node encode (bb::MenuConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_name);
			node.push_back(rhs.m_items);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfig & rhs)
		{
			try
			{
				rhs.m_name = node["name"].as<bbstring>();
				rhs.m_items = node["items"].as<std::vector<bb::MenuConfigItem>>();
			}
			catch (std::exception const & e)
			{
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
				return false;
			}
			return true;
		}
	};

	template<>
	struct convert<bb::MenuConfigItem>
	{
		static Node encode (bb::MenuConfigItem const & rhs)
		{
			Node node;
			node.push_back(rhs.m_name);
			node.push_back(static_cast<uint32_t>(rhs.m_type));
			node.push_back(rhs.m_value);
			if (rhs.m_menu)
				node.push_back(*rhs.m_menu);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfigItem & rhs)
		{
			try
			{
				rhs.m_name = node["name"].as<bbstring>();
				rhs.m_type = bb::e_MenuItemSeparator;
				if (node["type"])
				{
					rhs.m_type = static_cast<bb::MenuItemType>(node["type"].as<uint32_t>());
				}
				if (node["exec"])
				{
					rhs.m_value = node["exec"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemExec;
				}
				if (node["script"])
				{
					rhs.m_value = node["script"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemScript;
				}
				if (node["folder"])
				{
					rhs.m_value = node["folder"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemFolder;
				}
				if (node["menu"])
				{
					bb::MenuConfig m = node["menu"].as<bb::MenuConfig>();
					rhs.m_menu = std::move(std::unique_ptr<bb::MenuConfig>(new bb::MenuConfig(m)));
					rhs.m_type = bb::e_MenuItemMenu;
				}
			}
			catch (std::exception const & e)
			{
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
				return false;
			}
			return true;
		}
	};
}

namespace bb {

	bool loadMenuConfig (YAML::Node & y_root, MenuConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			if (YAML::Node y_menu = y_root["Menu"])
			{
				MenuConfig m = y_menu.as<MenuConfig>();
				config = std::move(m);
			}
			return true;
		}
		catch (std::exception & e)
		{
			TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
			return false;
		}
	}

}
