#pragma once
#include "WidgetConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::WidgetConfig>
	{
		static Node encode (bb::WidgetConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_id);
			node.push_back(rhs.m_widgetType);
			node.push_back(rhs.m_displayName);
			node.push_back(rhs.m_x);
			node.push_back(rhs.m_y);
			node.push_back(rhs.m_w);
			node.push_back(rhs.m_h);
			node.push_back(rhs.m_alpha);
			node.push_back(rhs.m_show);
			node.push_back(rhs.m_titlebar);
			return node;
		}

		static bool decode (Node const & node, bb::WidgetConfig & rhs)
		{
			try
			{
				rhs.m_id = node["id"].as<bbstring>();
				rhs.m_widgetType = node["widget"].as<bbstring>();
				if (node["name"])
					rhs.m_displayName = node["name"].as<bbstring>();
				if (node["x"])
					rhs.m_x = node["x"].as<int>();
				if (node["y"])
					rhs.m_y = node["y"].as<int>();
				if (node["w"])
					rhs.m_w = node["w"].as<int>();
				if (node["h"])
					rhs.m_h = node["h"].as<int>();
				if (node["alpha"])
					rhs.m_alpha = node["alpha"].as<int>();
				if (node["show"])
					rhs.m_show = node["show"].as<bool>();
				if (node["titlebar"])
					rhs.m_titlebar = node["titlebar"].as<bool>();
			}
			catch (std::exception const & e)
			{
				return false;
			}
			return true;
		}
	};
}

