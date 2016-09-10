#include "WidgetConfig.h"
#include "WidgetsConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::WidgetConfig>
	{
		static Node encode (bb::WidgetConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_widget);
			node.push_back(rhs.m_x);
			node.push_back(rhs.m_y);
			node.push_back(rhs.m_w);
			node.push_back(rhs.m_h);
			node.push_back(rhs.m_alpha);
			node.push_back(rhs.m_show);
			node.push_back(rhs.m_vertical);
			node.push_back(rhs.m_titlebar);
			return node;
		}

		static bool decode (Node const & node, bb::WidgetConfig & rhs)
		{
			try
			{
				rhs.m_widget = node["widget"].as<bbstring>();
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
				if (node["vertical"])
					rhs.m_vertical = node["vertical"].as<bool>();
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

namespace bb {

	bool loadWidgetsConfig (YAML::Node & y_root, WidgetsConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_widgets = y_root["Widgets"]; // @TODO: unicode? utf8?
			if (y_widgets)
			{
				int const n = y_widgets.size();
				for (int i = 0; i < n; ++i)
				{
					YAML::Node y_widgets_i = y_widgets[i];
					bb::WidgetConfig tmp = y_widgets[i].as<bb::WidgetConfig>();
					std::unique_ptr<WidgetConfig> tc(new WidgetConfig(tmp));
					config.m_widgets.push_back(std::move(tc));
				}
			}
		}
		catch (std::exception & e)
		{
			return false;
		}
	}

}
