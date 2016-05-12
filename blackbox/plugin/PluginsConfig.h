#pragma once
#include "PluginConfig.h"
#include <vector>
namespace YAML { class Node; }

namespace bb {

	struct PluginsConfig
	{
		std::vector<PluginConfig> m_plugins;
	};

	bool loadPluginsConfig (YAML::Node & y_root, PluginsConfig & config);
}
