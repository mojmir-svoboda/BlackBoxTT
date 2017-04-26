#pragma once
#include "MenuConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include <blackbox/gfx/MenuWidget.h>

namespace bb
{
	inline std::shared_ptr<MenuConfigItem> mkMenuConfigItem (YAML::Node node)
	{
		if (node["separator"])
		{
			return std::make_shared<MenuConfigItem>(e_MenuItemSeparator);
		}

		std::shared_ptr<MenuConfigItem> item;
		if (node["exec"])
		{
// 			rhs.m_value = node["exec"].as<bbstring>();
// 			rhs.m_type = bb::e_MenuItemExec;
			return std::make_shared<MenuConfigItem>(e_MenuItemSeparator);
		}
		else if (node["script"])
		{
			bbstring const name = node["name"].as<bbstring>();
			bbstring const script = node["script"].as<bbstring>();
			return std::make_shared<MenuConfigItemScript>(name, script);
		}
		else if (node["folder"])
		{
			bb::MenuConfigItemFolder fld = std::move(node.as<bb::MenuConfigItemFolder>());
			return std::make_shared<MenuConfigItemFolder>(std::move(fld));
		}
		else if (node["file"])
		{
			bb::MenuConfigItemFile fl = std::move(node.as<bb::MenuConfigItemFile>());
			return std::make_shared<MenuConfigItemFile>(std::move(fl));
		}
		else if (node["menu"])
		{
			bbstring const name = node["name"].as<bbstring>();
			YAML::Node y_submenu = node["menu"];
			std::shared_ptr<bb::MenuConfig> sub = std::make_shared<bb::MenuConfig>(y_submenu.as<bb::MenuConfig>());
			sub->m_widgetType = MenuWidget::c_type;
			return std::make_shared<MenuConfigItemSubMenu>(name, sub);
		}
		else if (node["checkbox"])
		{
			MenuConfigItemCheckBox chkbox = node.as<bb::MenuConfigItemCheckBox>();
			return std::make_shared<MenuConfigItemCheckBox>(chkbox);
		}
		return nullptr;
	}
}

namespace YAML {



	template<>
	struct convert<bb::MenuConfig>
	{
		static Node encode (bb::MenuConfig const & rhs)
		{
			Node node = convert<bb::WidgetConfig>::encode(rhs);
			//node.push_back(rhs.m_items);
			Assert(0 && "todo");
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfig & rhs)
		{
			try
			{
				if (convert<bb::WidgetConfig>::decode(node, rhs))
				{
					if (Node y_items = node["items"])
					{
						int const n = y_items.size();
						Assert(y_items.Type() == NodeType::Sequence);
						for (int i = 0; i < n; ++i)
						{
							YAML::Node y_item_i = y_items[i];
							std::shared_ptr<bb::MenuConfigItem> item_i = bb::mkMenuConfigItem(y_item_i);
							rhs.m_items.push_back(item_i);
						}
					}

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
			node.push_back(static_cast<uint32_t>(rhs.m_type));
			node.push_back(rhs.m_name);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfigItem & rhs)
		{
			try
			{
				if (node["type"])
					rhs.m_type = static_cast<bb::MenuItemType>(node["type"].as<uint32_t>());
				if (node["name"])
					rhs.m_name = node["name"].as<bbstring>();
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
	struct convert<bb::MenuConfigItemSubMenu>
	{
		static Node encode (bb::MenuConfigItemSubMenu const & rhs)
		{
			Node node = convert<bb::MenuConfigItem>::encode(rhs);
			if (rhs.m_menu)
				node.push_back(*rhs.m_menu);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfigItemSubMenu & rhs)
		{
			try
			{
				if (convert<bb::MenuConfigItem>::decode(node, rhs))
				{
					bb::MenuConfig m = node.as<bb::MenuConfig>();
					rhs.m_menu = std::move(std::unique_ptr<bb::MenuConfig>(new bb::MenuConfig(std::move(m))));
					rhs.m_type = bb::e_MenuItemSubMenu;
					if (rhs.m_menu->m_widgetType.empty())
						rhs.m_menu->m_widgetType = bb::MenuWidget::c_type;
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
					if (rhs.m_name.empty())
						rhs.m_name = node["checkbox"].as<bbstring>();
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

	template<>
	struct convert<bb::MenuConfigItemFolder>
	{
		static Node encode (bb::MenuConfigItemFolder const & rhs)
		{
			Node node = convert<bb::MenuConfigItemSubMenu>::encode(rhs);
			node.push_back(rhs.m_folderName);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfigItemFolder & rhs)
		{
			try
			{
				convert<bb::MenuConfigItemSubMenu>::decode(node, rhs);
				if (node["folder"])
				{
					rhs.m_folderName = node["folder"].as<bbstring>();
					if (rhs.m_menu && rhs.m_menu->m_id.empty())
						rhs.m_menu->m_id = rhs.m_folderName;
					rhs.InitFromExplorer();
					rhs.m_type = bb::e_MenuItemFolder;
				}
				else
				{
					TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML error in source %s: need section folder to configure folder", __FILE__);
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
	struct convert<bb::MenuConfigItemFile>
	{
		static Node encode (bb::MenuConfigItemFile const & rhs)
		{
			Node node = convert<bb::MenuConfigItem>::encode(rhs);
			node.push_back(rhs.m_fileName);
			return node;
		}

		static bool decode (Node const & node, bb::MenuConfigItemFile & rhs)
		{
			try
			{
				if (convert<bb::MenuConfigItem>::decode(node, rhs))
				{
					if (node["file"])
					{
						rhs.m_fileName = node["file"].as<bbstring>();
						rhs.InitFromExplorer();
						rhs.m_type = bb::e_MenuItemFile;
					}
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


