#pragma once
#include <vector>
#include <bblib/bbstring.h>
namespace YAML { class Node; }

namespace bb {

	struct WorkSpaceConfig
	{
		uint32_t m_vertex;
		bbstring m_id;
		bbstring m_label;

		uint32_t GetVertex () const { return m_vertex; }
		bbstring const & GetId () const { return m_id; }
	};

	struct WorkGraphConfig
	{
		bbstring m_id;
		bbstring m_label;
		std::vector<std::vector<bbstring>> m_vertexlists;
		std::vector<bbstring> m_edgelist;
		bbstring m_hotkey;
		bbstring m_currentVertexId;
		bbstring m_initVertexId;

		bool HasVertex (bbstring const & vertex_id) const
		{
			for (auto const & outer : m_vertexlists)
				for (bbstring const & v : outer)
					if (v == vertex_id)
						return true;
			return false;
		}
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

