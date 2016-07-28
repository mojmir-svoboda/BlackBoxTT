#pragma once
#include "WorkSpacesGraph.h"

namespace bb {

	struct VertexAdapter : std::binary_function<WorkSpacesGraph::vertex_t, WorkSpacesGraph::vertex_t, bool>
	{
		csr::vertex_t get_vertex (WorkSpacesGraph::vertex_t const & v) const { return v->GetVertex(); }
		// std::sorting
		bool operator() (WorkSpacesGraph::vertex_t const & lhs, WorkSpacesGraph::vertex_t const & rhs) const { return get_vertex(lhs) < get_vertex(rhs); }
		bool operator() (WorkSpacesGraph::edge_t const & lhs, WorkSpacesGraph::edge_t const & rhs) const {
			csr::vertex_pair_t v0(get_vertex(lhs.first), get_vertex(lhs.second));
			csr::vertex_pair_t v1(get_vertex(rhs.first), get_vertex(rhs.second));
			return v0 < v1;
		}
	};

	WorkSpacesGraph::WorkSpacesGraph () : m_Graph() { }

	void WorkSpacesGraph::Clear ()
	{
		m_Vertices.clear();
		m_Edges.clear();
		m_Graph.Clear();
	}

	/**@fn      CreateGraph
	 * @brief   creates csr graph from user supplied vertex list and edge list
	 *
	 * @return  true if graph can be constructed
	 */
	bool WorkSpacesGraph::CreateGraph ()
	{
		// vertex size limited to 16bit (0-65535)
		//assert(m_Vertices.Size() < std::numeric_limits<csr::vertex_t>::max());
		//DoAssert(m_Edges.Size() < std::numeric_limits<csr::vertex_t>::max());

		// compressed sparse row graph construction
		VertexAdapter adapter;
		std::sort(&m_Edges[0], &m_Edges[0] + m_Edges.size(), adapter); // sparse row format needs edges sorted by source vertex field
		bool const res = m_Graph.Construct(m_Vertices.size(), m_Edges.size(), &m_Edges[0], adapter); // construct compressed sparse row format
		return res;
	}

	/**@fn      WriteGraphToDotFile
	 * @brief   writes graph to .dot file (graphviz format)
	 *
	 * you can then render the .dot file using something like
	 *    c:/Program\ Files\ \(x86\)/Graphviz2.38/bin/dot -Tpng items.dot > items.png
	 * you can get graphviz from
	 *    http://www.graphviz.org/Download_windows.php
	 **/
	bool WorkSpacesGraph::WriteGraphToDotFile (char const * fname) const
	{
// 		FILE * f = fopen(fname, "w");
// 		if (!f)
// 			return false;
// 		fprintf(f, "digraph G {\n");
// 		for (size_t u = 0, ue = m_Graph.NumVertices(); u < ue; ++u)
// 			fprintf(f, "%u[label=\"%s\"];\n", (unsigned)u, m_Vertices[(int)u]->object->GetDebugName().Data());
// 		for (csr::vertex_t u = 0, ue = m_Graph.NumVertices(); u < ue; ++u) {
// 			csr::SparseGraph::edge_descriptor_pair_t const out_range = m_Graph.OutEdges(u);
// 			for (csr::SparseGraph::EdgeDescriptor ei = out_range.first; ei != out_range.second; ++ei.m_idx)
// 				fprintf(f, "%u->%u ;\n", (unsigned)u, m_Graph.Target(ei));
// 		}
// 		fprintf(f, "}\n");
//		fclose(f);
		return true;
	}
}
