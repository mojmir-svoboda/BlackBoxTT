#include "TrayConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::TrayConfig>
	{
		static Node encode (bb::TrayConfig const & rhs)
		{
			Node node;
// 			node.push_back(rhs.m_caption);
// 			node.push_back(rhs.m_ignored);
// 			node.push_back(rhs.m_sticky);
			return node;
		}

		static bool decode (Node const & node, bb::TrayConfig & rhs)
		{
			try
			{
// 				rhs.m_caption = node["window"].as<bbstring>();
// 				rhs.m_caption_regex = bbregex(rhs.m_caption);
// 				if (node["ignored"])
// 					rhs.m_ignored = node["ignored"].as<bool>();
// 				if (node["sticky"])
// 					rhs.m_sticky = node["sticky"].as<bool>();
			}
// 			catch (std::regex_error const & e)
// 			{
// 				//err.what();
// 				return false;
// 			}
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

	bool loadTrayConfig (YAML::Node & y_root, TrayConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_tasks = y_root["Tray"]; // @TODO: unicode? utf8?
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
