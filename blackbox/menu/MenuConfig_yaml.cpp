#include "MenuInfo.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::MenuConfig>
	{
		static Node encode (bb::MenuConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_caption);
			return node;
		}

		static bool decode (Node const & node, bb::TaskConfig & rhs)
		{
			try
			{
				rhs.m_caption = node["window"].as<bbstring>();
				if (node["workspace"])
					rhs.m_wspace = node["workspace"].as<bbstring>();
				rhs.m_caption_regex = bbregex(rhs.m_caption);
				if (node["bbtasks"])
					rhs.m_bbtasks = node["bbtasks"].as<bool>();
				if (node["taskman"])
					rhs.m_taskman = node["taskman"].as<bool>();
				if (node["sticky"])
					rhs.m_sticky = node["sticky"].as<bool>();
			}
			catch (std::regex_error const & e)
			{
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "regexp exception in source %s: %s", __FILE__, e.what());
				return false;
			}
			catch (std::exception const & e)
			{
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
				return false;
			}
			return true;
		}
	};
}

namespace bb {

	bool loadMenuConfig (YAML::Node & y_root, MenuConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_tasks = y_root["Menus"];
			if (y_tasks)
			{
				int const n = y_tasks.size();
				for (int i = 0; i < n; ++i)
				{
					YAML::Node y_tasks_i = y_tasks[i];
					bb::MenuConfig tmp = y_tasks[i].as<bb::MenuConfig>();
					std::unique_ptr<MenuConfig> cfg(new MenuConfig(tmp));
					config.m_tasks.push_back(std::move(cfg));
				}
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
