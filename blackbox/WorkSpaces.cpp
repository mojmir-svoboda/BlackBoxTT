#include "WorkSpaces.h"
#include "WorkSpacesConfig.h"
#include "BlackBox.h"
#include <bblib/logging.h>
#include <regex>
#include <tuple>
namespace bb {

	WorkSpaces::WorkSpaces ()
	{ }

	WorkSpaces::~WorkSpaces()
	{
	}

	bool WorkSpaces::Init (WorkSpacesConfig & config)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
		m_config = config;

		std::unique_ptr<VirtualDesktopManager> v(new VirtualDesktopManager);
		m_vdm = std::move(v);

		bool ok = true;
		size_t const l = m_graph.FindPropertyIndex(L"left");
		size_t const r = m_graph.FindPropertyIndex(L"right");
		ok &= m_vdm->Init(l, r);
		ok &= CreateGraph();
		InitClusterAndVertex();
		return ok;
	}

	void WorkSpaces::InitClusterAndVertex ()
	{
		if (m_config.m_currentClusterId.empty())
		{
			if (m_config.m_clusters.size())
			{
				SetCurrentClusterId(m_config.m_clusters[0].m_id);
			}
			else
			{
				// @TODO: error
			}
		}

		for (WorkGraphConfig & wg : m_config.m_clusters)
		{
			if (wg.m_auto)
			{
				GUID g = {0};
				if (m_vdm->GetCurrentDesktop(g))
				{
					size_t idx = 0;
					if (m_vdm->FindDesktop(g, idx))
					{
						wg.m_currentVertexId = m_vdm->m_names[idx];
					}
				}
			}
			else if (wg.m_currentVertexId.empty())
			{
				if (wg.m_vertexlists.size() > 0 && wg.m_vertexlists[0].size())
				{
					wg.m_currentVertexId = wg.m_vertexlists[0][0];
				}
				else
				{
					// @TODO: error
				}
			}
		}
	}

	void WorkSpaces::OnWindowCreated ()
	{
	}
	void WorkSpaces::OnWindowDestroyed ()
	{
	}
	void WorkSpaces::OnGraphConfigurationChanged ()
	{
		m_vdm->UpdateDesktopGraph();
		RecreateGraph();
	}

	bbstring const * WorkSpaces::GetCurrentVertexId () const
	{
		if (WorkGraphConfig const * cfg = FindCluster(GetCurrentClusterId()))
		{
			return &cfg->m_currentVertexId;
		}
		return nullptr;
	}

	void WorkSpaces::SetCurrentClusterId (bbstring const & id)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "Set wspace vertex: %ws", id.c_str());
		m_config.m_currentClusterId = id;
	}

	bool WorkSpaces::IsVertexVDM (bbstring const & vertex_id) const
	{
		size_t idx = 0;
		return m_vdm->FindDesktop(vertex_id, idx);
	}
	bool WorkSpaces::IsVertexVDM (bbstring const & vertex_id, size_t & idx) const
	{
		return m_vdm->FindDesktop(vertex_id, idx);
	}

	GUID WorkSpaces::GetVertexGUID (size_t idx) const
	{
		return m_vdm->m_desktops[idx];
	}

	bool WorkSpaces::SwitchDesktop (bbstring const & vertex_id)
	{
		size_t idx = 0;
		if (m_vdm->FindDesktop(vertex_id, idx))
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "VDM switch to vertex: %ws", vertex_id.c_str());
			m_vdm->SwitchDesktop(m_vdm->m_desktops[idx]);
			return true;
		}
		return false;
	}

	bool WorkSpaces::SetCurrentVertexId (bbstring const & vertex_id)
	{
		if (WorkGraphConfig * cfg = FindClusterForVertex(vertex_id))
		{
			cfg->m_currentVertexId = vertex_id;
			return true;
		}
		return false;
	}

	bool WorkSpaces::CanSetCurrentVertexId (bbstring const & vertex_id) const
	{
		if (WorkGraphConfig const * const cfg = FindClusterForVertex(vertex_id))
			return true;
		return false;
	}

	WorkGraphConfig const * WorkSpaces::FindClusterForVertex (bbstring const & vertex_id) const
	{
		for (size_t i = 0, ie = m_config.m_clusters.size(); i < ie; ++i)
			if (m_config.m_clusters[i].HasVertex(vertex_id))
				return &m_config.m_clusters[i];
		return nullptr;
	}
	WorkGraphConfig * WorkSpaces::FindClusterForVertex (bbstring const & vertex_id)
	{
		for (size_t i = 0, ie = m_config.m_clusters.size(); i < ie; ++i)
			if (m_config.m_clusters[i].HasVertex(vertex_id))
				return &m_config.m_clusters[i];
		return nullptr;
	}

	WorkGraphConfig const * WorkSpaces::FindCluster (bbstring const & cluster_id) const
	{
		for (size_t i = 0, ie = m_config.m_clusters.size(); i < ie; ++i)
			if (m_config.m_clusters[i].m_id == cluster_id)
				return &m_config.m_clusters[i];
		return nullptr;
	}
	WorkGraphConfig * WorkSpaces::FindCluster (bbstring const & cluster_id)
	{
		for (size_t i = 0, ie = m_config.m_clusters.size(); i < ie; ++i)
			if (m_config.m_clusters[i].m_id == cluster_id)
				return &m_config.m_clusters[i];
		return nullptr;
	}

	bool WorkSpaces::CanSwitchVertexViaEdge (bbstring const & edge_id, bbstring & target_vertex_id) const
	{
		WorkGraphConfig const * const w0 = FindCluster(m_config.m_currentClusterId);
		if (w0)
		{
			bbstring const & current_vertex_id = w0->m_currentVertexId;
				
			csr::vertex_t idx = 0;
			if (m_graph.FindVertexIndex(current_vertex_id, idx)) // @NOTE: O(N) can be avoided @TODO @FIXME
			{
				auto edges = m_graph.m_graph.OutEdges(idx);
				//for (uint16_t i = edges.first.m_idx; i < edges.second.m_idx; ++i)

				csr::vertex_t const v_row_start = m_graph.m_graph.m_rowstart[idx];
				csr::vertex_t const next_row_start = m_graph.m_graph.m_rowstart[idx + 1];
				csr::vertex_t const next_row_end = std::max(v_row_start, next_row_start);
				for (uint16_t i = v_row_start; i < next_row_end; ++i)
				{
					csr::vertex_t target = m_graph.m_graph.m_column[i];
					int n = m_graph.m_graph.m_props[i];
					bbstring const & name = m_graph.m_edgeProps[n];

					if (name == edge_id)
					{
						target_vertex_id = m_graph.m_vertices[target]->m_id;
						return true;
					}
				}
			}
		}

		return false;
	}

	bool WorkSpaces::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_WSPACE, "Terminating workspaces");
		ClearGraph();
		m_vdm->Done();
		m_vdm.release();
		return true;
	}

	bool WorkSpaces::CreateGraph ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_WSPACE, "Creating workspace graph...");
		// setup VDM first
		for (WorkGraphConfig & w : m_config.m_clusters)
		{
			if (w.m_auto)
			{
				w.m_vertexlists.push_back(std::vector<bbstring>());

				for (size_t i = 0, ie = m_vdm->m_names.size(); i < ie; ++i)
				{
					bbstring const & v = m_vdm->m_names[i];
					w.m_vertexlists[0].push_back(v);

					std::unique_ptr<WorkSpaceConfig> ws(new WorkSpaceConfig);
					ws->m_id = v;
					ws->m_label = v;
					ws->m_isVDM = true;
					ws->m_idxVDM = i;
					ws->m_vertex = m_graph.m_vertices.size();
					TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "Found VDM vertex: %ws", v.c_str());
					m_graph.m_vertices.push_back(std::move(ws));
				}

				WorkSpaceConfig * wspace = nullptr;
				for (auto const & e : m_vdm->m_edges)
				{
					WorkSpaceConfig * ws[2] = { 0 };
					uint32_t const src = std::get<0>(e);
					uint32_t const ep =  std::get<1>(e);
					uint32_t const dst = std::get<2>(e);
					if (m_graph.FindVertex(m_vdm->m_names[src], ws[0]) && m_graph.FindVertex(m_vdm->m_names[dst], ws[1]))
					{
						unsigned count = 0;
						m_graph.m_edges.push_back(std::make_tuple(ws[0], ep, ws[1])); // src ---label_idx---> dst
						TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "Found edge: %ws --> %ws", ws[0]->m_id.c_str(), ws[1]->m_id.c_str());
					}
				}
			}
		}

		for (WorkGraphConfig & w : m_config.m_clusters)
		{
			for (auto const & vtxlist : w.m_vertexlists)
			{
				for (bbstring const & s : vtxlist)
				{
					std::unique_ptr<WorkSpaceConfig> ws(new WorkSpaceConfig);
					ws->m_id = s;
					ws->m_label = s;
					ws->m_vertex = m_graph.m_vertices.size();
					m_graph.m_vertices.push_back(std::move(ws));
					TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "Found config vertex: %ws", s.c_str());
				}
			}
		}
		for (WorkGraphConfig & w : m_config.m_clusters)
		{
			for (bbstring const & s : w.m_edgelist)
			{
				try
				{
					std::wregex label_regex(L"(.+)\\[\\s*label\\s*=\\s*\"(.+)\"\\s*\\]");
					std::wsmatch label_match;
					if (std::regex_match(s, label_match, label_regex))
					{
						if (label_match.size() == 3)
						{
							// parse label of the edge
							std::wssub_match sub_match2 = label_match[2];
							bbstring label = sub_match2.str();

							// parse edge list
							std::wssub_match sub_match1 = label_match[1];
							bbstring edgelist = sub_match1.str();

							std::wregex edges_regex(L"\\s*->\\s*");
							std::wsregex_token_iterator it(edgelist.begin(), edgelist.end(), edges_regex, -1);
							std::wsregex_token_iterator reg_end;

							WorkSpaceConfig * ws[2] = { 0 };
							unsigned count = 0;

							for (; it != reg_end; ++it)
							{
								bbstring v = it->str();
								v.erase(std::remove(v.begin(), v.end(), L' '), v.end());
								WorkSpaceConfig * wspace = nullptr;
								if (m_graph.FindVertex(v, wspace))
								{
									ws[count++] = wspace;
								}
								else
								{
									TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "edgelist error: cannot find vertex: %s", v.c_str());
									m_graph.Clear();
									return false;
								}

								if (count == 2)
								{
									size_t const n = m_graph.FindPropertyIndex(label);
									m_graph.m_edges.push_back(std::make_tuple(ws[0], n, ws[1])); // src ---label_idx---> dst
									TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "Found edge: %ws --> %ws", ws[0]->m_id.c_str(), ws[1]->m_id.c_str());
									ws[0] = ws[1];
									count = 1;
								}
							}
						}
						else
						{
							TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "error: cannot parse entry: %s", s.c_str());
							m_graph.Clear();
							return false;
						}
					}
				}
				catch (std::regex_error const & e)
				{
					TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "error: regex_error code=%d e.what=%s", e.code(), e.what());
					m_graph.Clear();
					return false;
				}

				catch (std::exception const & e)
				{
					TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "error: std::exception e.what=%s", e.what());
					m_graph.Clear();
					return false;
				}
			}
		}

		bool const graph_ok = m_graph.CreateGraph();
		TRACE_MSG((graph_ok ? LL_INFO : LL_ERROR), (CTX_BB | CTX_WSPACE), "Creating workspace graph... %s", graph_ok ? "ok" : "error");
		return graph_ok;
	}

	void WorkSpaces::ClearGraph ()
	{
		m_graph.Clear();
	}

	bool WorkSpaces::RecreateGraph ()
	{
		for (WorkGraphConfig & w : m_config.m_clusters)
		{
			if (w.m_auto)
			{
				w.m_vertexlists.clear();
				w.m_edgelist.clear();
				w.m_currentVertexId.clear();
			}
		}

		ClearGraph();

		const bool ok = CreateGraph();
		InitClusterAndVertex();
		return ok;
	}

	bool WorkSpaces::AssignWorkSpace (HWND hwnd, bbstring & vertex_id)
	{
		size_t idx = 0;
		if (m_vdm->FindDesktopIndex(hwnd, idx))
		{
			vertex_id = m_vdm->m_names[idx];
			return true;
		}

		if (bbstring const * current_ws = GetCurrentVertexId())
		{
			vertex_id = *current_ws;
			return true;
		}
		return false;
	}

}
