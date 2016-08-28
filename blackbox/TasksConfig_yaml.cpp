#include "TaskInfo.h"
#include "TasksConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::TaskConfig>
	{
		static Node encode (bb::TaskConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_caption);
			node.push_back(rhs.m_wspace);
			node.push_back(rhs.m_bbtasks);
			node.push_back(rhs.m_taskman);
			node.push_back(rhs.m_sticky);
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
				//err.what();
				return false;
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

	bool loadTasksConfig (YAML::Node & y_root, TasksConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_tasks = y_root["Tasks"]; // @TODO: unicode? utf8?
			if (y_tasks)
			{
				int const n = y_tasks.size();
				//YAML::NodeType::value cst = y_tasks.Type();
				for (int i = 0; i < n; ++i)
				{
					YAML::Node y_tasks_i = y_tasks[i];
					bb::TaskConfig tmp = y_tasks[i].as<bb::TaskConfig>();
					std::unique_ptr<TaskConfig> tc(new TaskConfig(tmp));
					config.m_tasks.push_back(std::move(tc));

					//YAML::NodeType::value cst = y_tasks_i.Type();
					//					std::string s = y_tasks_i["window"].as<std::string>();
					//					YAML::Node y_ignore = y_tasks_i["ignore"];
					//					if (y_ignore)
					//					{
					//						const bool ignore = y_ignore.as<bool>();
					//						OutputDebugStringA(s.c_str());
					//					}
				}
			}
		}
		catch (std::exception & e)
		{
			return false;
		}
	}

}
