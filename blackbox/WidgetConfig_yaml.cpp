#include "WidgetConfig.h"
#include "WidgetConfig_yaml.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace bb {

	bool loadWidgetConfig (YAML::Node & y_node, WidgetConfig & config)
	{
		try
		{
			if (y_node.IsNull())
				return false;

			config = y_node.as<bb::WidgetConfig>();
			return true;
		}
		catch (std::exception & e)
		{
			TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
			return false;
		}
		return false;
	}
}

