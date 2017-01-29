#include "MenuConfig.h"
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <yaml-cpp/yaml.h>
#include "MenuConfig_yaml.h"

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
