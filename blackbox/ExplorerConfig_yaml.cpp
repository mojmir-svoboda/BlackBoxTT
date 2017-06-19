#include "ExplorerConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::ExplorerConfig>
	{
		static Node encode (bb::ExplorerConfig const & rhs)
		{
			Node node;
 			node.push_back(rhs.m_show);
			return node;
		}

		static bool decode (Node const & node, bb::ExplorerConfig & rhs)
		{
			try
			{
 				if (node["show"])
 					rhs.m_show = node["show"].as<bool>();
				return true;
			}
			catch (std::exception const & e)
			{
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
				return false;
			}
			return false;
		}
	};
}

namespace bb {

	bool loadExplorerConfig (YAML::Node & y_root, ExplorerConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_tasks = y_root["Explorer"];
			if (y_tasks)
			{
				int const n = y_tasks.size();
				for (int i = 0; i < n; ++i)
				{
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
