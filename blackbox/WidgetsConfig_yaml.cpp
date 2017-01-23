#include "WidgetConfig.h"
#include "WidgetsConfig.h"
#include "WidgetConfig_yaml.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

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
				return true;
			}
		}
		catch (std::exception & e)
		{
			TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
			return false;
		}
		return false;
	}

}
