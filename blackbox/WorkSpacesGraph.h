#pragma once
#include <csr_graph/SparseGraph.h>
#include <WorkSpacesConfig.h>

namespace bb {

	struct WorkSpacesGraph
	{
		typedef WorkSpaceConfig * vertex_t;
		std::vector<std::unique_ptr<WorkSpaceConfig>> m_vertices;       /// vertices (objects) of the graph
		typedef std::tuple<vertex_t, int, vertex_t> edge_t;  // vertex1 --- int_property --> vertex2
		std::vector<edge_t> m_edges;            /// edges (dependencies) of the graph
		std::vector<bbstring> m_edgeProps;
		csr::SparseGraph m_graph;             /// compressed sparse row graph

		WorkSpacesGraph ();

		void Clear ();

		template<class IdT>
		bool FindVertex (IdT const & id, vertex_t & vtx)
		{
			for (std::unique_ptr<WorkSpaceConfig> const & v : m_vertices)
			{
				if (v->GetId() == id)
				{
					vtx = v.get();
					return true;
				}
			}
			return false;
		}

		/// prop created if not in container already
		size_t FindPropertyIndex (bbstring const & s)
		{
			for (size_t i = 0, ie = m_edgeProps.size(); i < ie; ++i)
				if (m_edgeProps[i] == s)
					return i;
			m_edgeProps.push_back(s);
			return m_edgeProps.size() - 1;
		}

		/**@fn      CreateGraph
		 * @brief   creates csr graph from user supplied vertex list and edge list
		 *
		 * @return  true if graph can be constructed
		 */
		bool CreateGraph ();

		/**@fn      WriteGraphToDotFile
		 * @brief   writes graph to .dot file (graphviz format)
		 *
		 * you can then render the .dot file using something like
		 *    c:/Program\ Files\ \(x86\)/Graphviz2.38/bin/dot -Tpng items.dot > items.png
		 * you can get graphviz from
		 *    http://www.graphviz.org/Download_windows.php
		 **/
		bool WriteGraphToDotFile (char const * fname) const;
	};
}
