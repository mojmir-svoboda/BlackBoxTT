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

		bool ok = true;
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
			if (wg.m_currentVertexId.empty())
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
		m_config.m_currentClusterId = id;
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

	bool WorkSpaces::SwitchVertex (bbstring const & new_vertex_id)
	{
		WorkGraphConfig * w0 = FindCluster(m_config.m_currentClusterId);
		WorkGraphConfig * w1 = FindClusterForVertex(new_vertex_id);
		if (w0 && w1)
		{
			bool const same_cluster = w0 == w1;
			bbstring const & current_vertex_id = w0->m_currentVertexId;

			BlackBox & bb = BlackBox::Instance();

			if (w0 != w1)
			{
				//OnSwitchCluster if needed
				SetCurrentClusterId(w1->m_id);
			}

			SetCurrentVertexId(new_vertex_id);
			bb.GetTasks().SwitchWorkSpace(current_vertex_id, new_vertex_id);
			//bb.GetTasks().ShowTasksFromWorkSpace(new_vertex_id);
		}
		return false;
	}

	bool WorkSpaces::SwitchVertexViaEdge (bbstring const & edge_property)
	{
		return false;
	}

	bool WorkSpaces::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB, "Terminating workspaces");
		ClearGraph();
		return true;
	}

	bool WorkSpaces::CreateGraph ()
	{
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
				}
			}
		}
		for (WorkGraphConfig & w : m_config.m_clusters)
		{
			for (bbstring const & s : w.m_edgelist)
			{
				//'f1 -> f2 -> f3 -> f1 [label="right"]'

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
		return graph_ok;
	}

	void WorkSpaces::ClearGraph ()
	{
		m_graph.Clear();
	}

}
