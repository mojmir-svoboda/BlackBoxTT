#include "PluginConfig.h"
#include "PluginsConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::PluginConfig>
	{
		static Node encode (bb::PluginConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_name);
			node.push_back(rhs.m_path);
			node.push_back(rhs.m_enabled);
			node.push_back(rhs.m_isSlit);
			node.push_back(rhs.m_inSlit);
			return node;
		}

		static bool decode (Node const & node, bb::PluginConfig & rhs)
		{
			try
			{
				rhs.m_name = node["plugin"].as<bbstring>();
				rhs.m_path = node["path"].as<bbstring>();
				if (node["enabled"])
					rhs.m_enabled = node["enabled"].as<bool>();
				if (node["is_slit"])
					rhs.m_isSlit = node["is_slit"].as<bool>();
				if (node["in_slit"])
					rhs.m_inSlit = node["in_slit"].as<bool>();
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

	bool loadPluginsConfig (YAML::Node & y_root, PluginsConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_cfg = y_root["Plugins"]; // @TODO: unicode? utf8?
			if (y_cfg)
			{
				int const n = y_cfg.size();
				//YAML::NodeType::value cst = y_tasks.Type();
				for (int i = 0; i < n; ++i)
				{
					YAML::Node y_cfg_i = y_cfg[i];
					bb::PluginConfig cfg = y_cfg[i].as<bb::PluginConfig>();
					config.m_plugins.push_back(cfg);
				}
			}
		}
		catch (std::exception & e)
		{
			return false;
		}
	}

}
