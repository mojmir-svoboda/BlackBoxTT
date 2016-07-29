#pragma once
#include <memory>
#include <algorithm>
#include <vector>

namespace csr {

	using vertex_t = uint16_t;
	using vertex_pair_t = std::pair<vertex_t, vertex_t>;

	struct SparseGraph
	{
		typedef std::vector<vertex_t> rowstart_t;
		typedef std::vector<vertex_t> column_t;
		typedef std::vector<int> edgeprops_t;

		rowstart_t m_rowstart;
		column_t   m_column;	// edge array (only target vertices of edges are stored)
		edgeprops_t m_props;

		/**@fn	  Construct
		 * @brief create Compressed Sparse Row format from edge list
		 *
		 * The CSR format stores vertices and edges in separate arrays, with the indices into these arrays corresponding
		 * to the identifier for the vertex or edge, respectively. The edge array is sorted by the source of each edge,
		 * but contains only the targets for the edges. The m_rowstart stores offsets into the edge array, providing
		 * the offset of the first edge outgoing from each vertex.
		 */
		template <class EdgeT, class AdapterT>
		bool Construct (int n_vertices, int n_edges, EdgeT const * edges, AdapterT const & adapter)
		{
			m_rowstart.resize(n_vertices + 1);
			m_column.reserve(n_edges);
			m_props.reserve(n_edges);
			vertex_t curr_edge = 0;
			vertex_t curr_vtx_plus1 = 1;
			m_rowstart[0] = 0;
			for (size_t ei = 0; ei < n_edges; ++ei)
			{
				vertex_t const src = adapter.get_vertex(std::get<0>(edges[ei]));
				vertex_t const tgt = adapter.get_vertex(std::get<2>(edges[ei]));
				for (; curr_vtx_plus1 != src + 1; ++curr_vtx_plus1)
					m_rowstart[curr_vtx_plus1] = curr_edge;
				m_column.push_back(tgt);
				m_props.push_back(std::get<1>(edges[ei]));
				++curr_edge;
			}

			for (; curr_vtx_plus1 != n_vertices + 1; ++curr_vtx_plus1)
				m_rowstart[curr_vtx_plus1] = curr_edge;
			return true;
		}

		struct EdgeDescriptor
		{
			vertex_t m_src;
			vertex_t m_idx;

			EdgeDescriptor (vertex_t s, vertex_t i): m_src(s), m_idx(i) {}
			EdgeDescriptor (): m_src(0), m_idx(0) {}

			bool operator== (EdgeDescriptor const & e) const { return m_idx == e.m_idx; }
			bool operator!= (EdgeDescriptor const & e) const { return m_idx != e.m_idx; }
			bool operator<  (EdgeDescriptor const & e) const { return m_idx <  e.m_idx; }
			bool operator>  (EdgeDescriptor const & e) const { return m_idx  > e.m_idx; }
			bool operator<= (EdgeDescriptor const & e) const { return m_idx <= e.m_idx; }
			bool operator>= (EdgeDescriptor const & e) const { return m_idx >= e.m_idx; }
		};
		typedef std::pair<EdgeDescriptor, EdgeDescriptor> edge_descriptor_pair_t;

		/**@fn	  OutEdges
		 * @brief returns pair of iterators
		 *
		 * Iteration over the out-edges for the ith vertex in
		 * the graph is achieved by visiting m_column[m_rowstart[i]], m_column[[m_rowstart[i]+1], ..., m_column[[i+1]].
		 */
		edge_descriptor_pair_t OutEdges (vertex_t v) const
		{
			vertex_t const v_row_start = m_rowstart[v];
			vertex_t const next_row_start = m_rowstart[v + 1];
			return std::make_pair(
					EdgeDescriptor(v, v_row_start),
					EdgeDescriptor(v, std::max(v_row_start, next_row_start)));
		}

		vertex_t Target (EdgeDescriptor e) const { return m_column[e.m_idx]; }
		int PropertyIndex (EdgeDescriptor e) const { return m_props[e.m_idx]; }
		vertex_pair_t Vertices () const { return std::make_pair(0, NumVertices()); }
		vertex_t NumVertices () const { return static_cast<vertex_t>(m_rowstart.size() - 1); }

		void Clear ()
		{
			m_rowstart.clear();
			m_column.clear();
			m_props.clear();
		}
	};
}

