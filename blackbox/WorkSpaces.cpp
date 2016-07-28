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
			}
		}
		for (WorkGraphConfig & w : m_config.m_clusters)
		{
			for (bbstring const & s : w.m_edgelist)
			{
				//'f1 -> f2 -> f3 -> f1 [label="right"]'

				try
				{
// 					std::wsmatch label_match;
// 					if (std::regex_match(s, label_match, label_regex))
// 					{
// 						for (size_t i = 0; i < label_match.size(); ++i)
// 						{
// 							std::wssub_match sub_match = label_match[i];
// 							bbstring piece = sub_match.str();
// 						
// 							piece.begin();
// 						}
// 					}

// 					std::wregex label_regex(L"\\[.*\\]");
// 					wchar_t tmp[1024] = { 0 };
// 					std::regex_replace(tmp, s.c_str(), s.c_str() + s.length(), label_regex, L"$&", std::regex_constants::format_no_copy);
// 					size_t const tmp_n = wcslen(tmp);
// 
// 					wchar_t tmp1[1024] = { 0 };
// 					std::wregex tag_regex(L"<((?!br).)*?>");
// 					std::regex_replace(tmp1, tmp, tmp + tmp_n, tag_regex, L"");
// 					size_t const tmp1_n = wcslen(tmp1);
// 
// 					std::wregex br_regex(L";");
// 					std::wcregex_token_iterator it(tmp1, tmp1 + tmp1_n, br_regex, -1);
// 					std::wcregex_token_iterator reg_end;
// 					for (; it != reg_end; ++it)
// 					{
// 						bbstring const & s = it->str().c_str();
// 					}

				}
				catch (std::regex_error const & e)
				{
					//std::cout << "regex pico: " << e.what() << "code=" << e.code() << std::endl;
				}

				catch (std::exception const & e)
				{
					//std::cout << "pico: " << e.what() << std::endl;
				}

			}
		}
		return true;
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
