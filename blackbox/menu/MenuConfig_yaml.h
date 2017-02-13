#pragma once
#include "MenuConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {


	template<>
	struct convert<bb::MenuConfig>
	{
		static Node encode (bb::MenuConfig const & rhs)
		{
			Node node = convert<bb::WidgetConfig>::encode(rhs);
			node.push_back(rhs.m_items);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfig & rhs)
		{
			try
			{
				if (convert<bb::WidgetConfig>::decode(node, rhs))
				{
					rhs.m_items = node["items"].as<std::vector<std::unique_ptr<bb::MenuConfigItem>>>();
					return true;
				}
			}
			catch (std::exception const & e)
			{
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
				return false;
			}
			return false;
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
				rhs.m_type = bb::e_MenuItemSeparator;
				if (node["separator"])
				{
					return true;
				}

				rhs.m_name = node["name"].as<bbstring>();
				if (node["exec"])
				{
					rhs.m_value = node["exec"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemExec;
				}
				else if (node["script"])
				{
					rhs.m_value = node["script"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemScript;
				}
				else if (node["folder"])
				{
					rhs.m_value = node["folder"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemFolder;
				}
				else if (node["menu"])
				{
					bb::MenuConfig m = node["menu"].as<bb::MenuConfig>();
					rhs.m_menu = std::move(std::unique_ptr<bb::MenuConfig>(new bb::MenuConfig(m)));
					rhs.m_type = bb::e_MenuItemMenu;
				}
				else if (YAML::Node n = node["checkbox"])
				{
					rhs = n.as<bb::MenuConfigItemCheckBox>();
					//rhs.m_type = bb::e_MenuItemCheckBox;
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

	template<>
	struct convert<bb::MenuConfigItemCheckBox>
	{
		static Node encode (bb::MenuConfigItemCheckBox const & rhs)
		{
			Node node = convert<bb::MenuConfigItem>::encode(rhs);
			node.push_back(rhs.m_getScript);
			node.push_back(rhs.m_onCheckScript);
			node.push_back(rhs.m_onUncheckScript);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfigItemCheckBox & rhs)
		{
			try
			{
				if (convert<bb::MenuConfigItem>::decode(node, rhs))
				{
					rhs.m_getScript = node["get"].as<bbstring>();
					rhs.m_onCheckScript = node["onCheck"].as<bbstring>();
					rhs.m_onUncheckScript = node["onUncheck"].as<bbstring>();
					rhs.m_type = bb::e_MenuItemCheckBox;
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


