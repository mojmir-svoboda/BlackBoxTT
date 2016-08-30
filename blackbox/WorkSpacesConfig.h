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
		bool m_auto { false };
		std::vector<std::vector<bbstring>> m_vertexlists;
		std::vector<bbstring> m_edgelist;
		bbstring m_currentVertexId;

		bool HasVertex (bbstring const & vertex_id) const
		{
			for (auto const & outer : m_vertexlists)
				for (bbstring const & v : outer)
					if (v == vertex_id)
						return true;
			return false;
		}

		uint32_t MaxColCount () const
		{
			uint32_t c = 0;
			for (auto const & outer : m_vertexlists)
			{
				uint32_t const o_n = static_cast<uint32_t>(outer.size());
				if (o_n > c)
					c = o_n;
			}
			return c;
		}

		uint32_t MaxRowCount () const
		{
			uint32_t const n = static_cast<uint32_t>(m_vertexlists.size());
			return n;
		}
	};

	struct WorkSpacesConfig
	{
		bbstring m_currentClusterId;
    std::vector<WorkGraphConfig> m_clusters;
		std::vector<std::string> m_edgelist;
		// cluster edges
	};

	bool loadWorkSpacesConfig (YAML::Node & y_root, WorkSpacesConfig & config);
}

