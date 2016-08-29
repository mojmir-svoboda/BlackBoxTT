#include "TaskInfo.h"
#include "WorkSpacesConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

struct WorkGraphConfig
{
	bbstring m_id;
	bbstring m_label;
	bbstring m_hotkey;
	bbstring m_currentVertexId;
	std::vector<std::string> m_verticesCfg;
	std::vector<std::string> m_edgesCfg;
};

struct WorkSpacesConfig
{
	bbstring m_currentClusterId;
	bbstring m_initClusterId;
	std::vector<WorkGraphConfig> m_groups;
	// cluster edges
};


namespace YAML {
	template<>
	struct convert<bb::WorkGraphConfig>
	{
		static Node encode (bb::WorkGraphConfig const & rhs)
		{
			Node node;
			node.push_back(rhs.m_id);
			node.push_back(rhs.m_label);
			node.push_back(rhs.m_hotkey);
			node.push_back(rhs.m_currentVertexId);
			node.push_back(rhs.m_vertexlists);
			node.push_back(rhs.m_edgelist);
			return node;
		}

		static bool decode (Node const & node, bb::WorkGraphConfig & rhs)
		{
			try
			{
				rhs.m_id = node["id"].as<bbstring>();
				if (node["label"])
					rhs.m_label = node["label"].as<bbstring>();
				if (node["hotkey"])
					rhs.m_hotkey = node["hotkey"].as<bbstring>();
				if (node["vertexlist"])
					rhs.m_vertexlists = node["vertexlist"].as<std::vector<std::vector<bbstring>>>();
				if (node["edgelist"])
					rhs.m_edgelist = node["edgelist"].as<std::vector<bbstring>>();
				if (node["current"])
					rhs.m_currentVertexId = node["current"].as<bbstring>();


//         YAML::Node y_wspaces = node["spaces"];
//         if (y_wspaces)
//         {
//           int const n = y_wspaces.size();
//           for (int i = 0; i < n; ++i)
//           {
//             YAML::Node y_wspaces_i = y_wspaces[i];
//             bb::WorkSpaceConfig cfg = y_wspaces[i].as<bb::WorkSpaceConfig>();
//             rhs.m_wspaces.push_back(cfg);
//           }
//         }
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

	bool loadWorkSpacesConfig (YAML::Node & y_root, WorkSpacesConfig & rhs)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node node = y_root["WorkSpaces"]; // @TODO: unicode? utf8?

			if (node["current"])
				rhs.m_currentClusterId = node["current"].as<bbstring>();
			if (node["edgelist"])
				rhs.m_edgelist = node["edgelist"].as<std::vector<std::string>>();

			YAML::Node y_clusters = node["clusters"]; // @TODO: unicode? utf8?
			if (y_clusters)
			{
				int const n = y_clusters.size();
				//YAML::NodeType::value cst = y_tasks.Type();
				for (int i = 0; i < n; ++i)
				{
					YAML::Node y_clusters_i = y_clusters[i];
					bb::WorkGraphConfig cfg = y_clusters_i.as<bb::WorkGraphConfig>();
					rhs.m_clusters.push_back(cfg);
				}
			}
		}
		catch (std::exception & e)
		{
			return false;
		}
	}

}
