#pragma once
#include <csr_graph/SparseGraph.h>
#include <WorkSpacesConfig.h>

namespace bb {

	struct WorkSpacesGraph
	{
		typedef bb::WorkSpaceConfig * vertex_t;
		std::vector<std::unique_ptr<vertex_t>> m_Vertices;       /// vertices (objects) of the graph
		typedef std::pair<vertex_t, vertex_t> edge_t;
		std::vector<edge_t> m_Edges;            /// edges (dependencies) of the graph
		csr::SparseGraph m_Graph;             /// compressed sparse row graph

		WorkSpacesGraph ();

		void Clear ();

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
