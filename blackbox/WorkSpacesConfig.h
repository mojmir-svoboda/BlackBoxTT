#pragma once
#include <vector>
#include <bblib/bbstring.h>
namespace YAML { class Node; }

namespace bb {

	struct WorkGraphConfig
	{
		bbstring m_id;
		bbstring m_label;
		std::vector<std::vector<std::string>> m_vertexlists;
		std::vector<std::string> m_edgelist;
		bbstring m_hotkey;
		bbstring m_currentVertexId;
		bbstring m_initVertexId;
	};

	struct WorkSpacesConfig
	{
		bbstring m_currentClusterId;
		bbstring m_initClusterId;
    std::vector<WorkGraphConfig> m_clusters;
		std::vector<std::string> m_edgelist;
		// cluster edges
	};

	bool loadWorkSpacesConfig (YAML::Node & y_root, WorkSpacesConfig & config);
}

