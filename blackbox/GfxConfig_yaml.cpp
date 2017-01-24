#include "GfxConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::GfxConfig>
	{
		static Node encode (bb::GfxConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_use);
			node.push_back(rhs.m_startWidgets);
			return node;
		}

		static bool decode (Node const & node, bb::GfxConfig & rhs)
		{
			try
			{
				rhs.m_use = node["use"].as<bbstring>();
				rhs.m_startWidgets = node["start"].as<std::vector<bbstring>>();
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

	bool loadGfxConfig (YAML::Node & y_root, GfxConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_gfx = y_root["Gfx"];
			if (y_gfx)
			{
				config = y_gfx.as<GfxConfig>();
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
