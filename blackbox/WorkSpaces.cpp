#include "WorkSpaces.h"
#include "WorkSpacesConfig.h"
#include <bblib/logging.h>
#include <regex>
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
					std::wregex label_regex(L"(.+)\\[(.+)\\]");
					std::wsmatch label_match;
					if (std::regex_match(s, label_match, label_regex))
					{
						if (label_match.size() == 3)
						{
							// parse label of the edge
							std::wssub_match sub_match2 = label_match[2];
							bbstring label = sub_match2.str();

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
									m_graph.m_edges.push_back(std::make_pair(ws[0], ws[1]));
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

	bool WorkSpaces::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB, "Terminating workspaces");
		ClearGraph();
		return true;
	}

	bool WorkSpaces::CreateGraph ()
	{
		
		return true;
	}

	void WorkSpaces::ClearGraph ()
	{
	}
}
