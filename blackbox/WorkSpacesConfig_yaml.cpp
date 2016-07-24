#include "TaskInfo.h"
#include "WorkSpacesConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::WorkSpaceConfig>
	{
		static Node encode (bb::WorkSpaceConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_name);
			return node;
		}

		static bool decode (Node const & node, bb::WorkSpaceConfig & rhs)
		{
			try
			{
				rhs.m_name = node["name"].as<bbstring>();
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

namespace YAML {
	template<>
	struct convert<bb::WorkGroupConfig>
	{
		static Node encode (bb::WorkGroupConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_name);
			node.push_back(rhs.m_x);
			node.push_back(rhs.m_y);
      node.push_back(rhs.m_wspaces);
			return node;
		}

		static bool decode (Node const & node, bb::WorkGroupConfig & rhs)
		{
			try
			{
				rhs.m_name = node["name"].as<bbstring>();
				rhs.m_hotkey = node["hotkey"].as<bbstring>();
				rhs.m_x = node["x"].as<uint16_t>();
				rhs.m_y = node["y"].as<uint16_t>();

        YAML::Node y_wspaces = node["spaces"];
        if (y_wspaces)
        {
          int const n = y_wspaces.size();
          for (int i = 0; i < n; ++i)
          {
            YAML::Node y_wspaces_i = y_wspaces[i];
            bb::WorkSpaceConfig cfg = y_wspaces[i].as<bb::WorkSpaceConfig>();
            rhs.m_wspaces.push_back(cfg);
          }
        }
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

	bool loadWorkSpacesConfig (YAML::Node & y_root, WorkSpacesConfig & config)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node y_wspaces = y_root["WorkGroups"]; // @TODO: unicode? utf8?
			if (y_wspaces)
			{
				int const n = y_wspaces.size();
				//YAML::NodeType::value cst = y_tasks.Type();
				for (int i = 0; i < n; ++i)
				{
					YAML::Node y_wspaces_i = y_wspaces[i];
					bb::WorkGroupConfig cfg = y_wspaces_i.as<bb::WorkGroupConfig>();
					config.m_groups.push_back(cfg);
				}
			}
		}
		catch (std::exception & e)
		{
			return false;
		}
	}

}
