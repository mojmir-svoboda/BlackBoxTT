#include "rc_cache.h"
#include <map>
#include <boost/variant/get.hpp>
#include <boost/algorithm/string.hpp>


//#define LOWER_KEY

namespace rc {

	static ParsedFileCache g_cache;
	ParsedFileCache const & getParsedFileCacheConst () { return g_cache; }
	ParsedFileCache & getParsedFileCache () { return g_cache; }

	namespace index {

		struct icasecompare : std::binary_function<tstring, tstring, bool> {
			bool operator() (tstring const & lhs, tstring const & rhs) const {
				return _tcsicmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};

		void makeIndex (trie_t & trie, props_t & props)
		{
			try
			{
				props_t tmp_props;
				tmp_props.reserve(1024);
				std::map<tstring, int, icasecompare> tmp_propmap;
				for (auto & item : getParsedFileCache().m_cache)
				{
					tstring const & rcfile = item.first;
					ParsedFileRecord & rec = item.second;

					for (size_t i = 0, ie = rec.m_parsedFile.size(); i < ie; ++i)
					{
						rc::line_type & line = rec.m_parsedFile[i];
						if (line.which() == e_line_pair)
						{
							pair_type & p = boost::get<pair_type &>(line);
							key_type const & key = p.first;
							value_type const & val = p.second;

							std::map<tstring, int>::iterator it = tmp_propmap.find(key);
							if (it == tmp_propmap.end())
							{
								tstring key_lwr = key;
#if defined LOWER_KEY
								boost::algorithm::to_lower(key_lwr);
#endif

								tmp_props.push_back(Props(key_lwr, i, rcfile));
								trie_t::result_type const id = static_cast<trie_t::result_type>(tmp_props.size() - 1);
								tmp_propmap[key_lwr] = id;
								wprintf(TEXT("insert: key=%s rcfile=%s idx=%i\n"), key_lwr.c_str(), rcfile.c_str(), id);
								trie.update(key_lwr.c_str(), key_lwr.length(), id);
							}
							else
							{
								tmp_props[it->second].m_rcfiles.push_back(rcfile);
								tmp_props[it->second].m_indices.push_back(i);
								wprintf(TEXT("update: key=%s rcfile=%s idx=%i\n"), key.c_str(), rcfile.c_str(), it->second);
							}

						}
					}

					/*SearchDirectory(
						  info.m_dir_path, info.m_includes, info.m_excludes, info.m_recursive, info.m_follow_symlinks
						, TEXT("")
						, [] (tstring const & fname, tstring const & cmp) { return true; }
						, [&trie, &tmp_props, &tmp_propmap] (tstring const & fname, tstring const & fpath)
							 {
								std::map<tstring, int>::iterator it = tmp_propmap.find(fname);
								if (it == tmp_propmap.end())
								{
									tstring fname_lwr = fname;
									boost::algorithm::to_lower(fname_lwr);
									tmp_props.push_back(Props(fname_lwr, fpath));
									trie_t::result_type const id = static_cast<trie_t::result_type>(tmp_props.size() - 1);
									tmp_propmap[fname_lwr] = id;
									//dbg_printf("insert: fname=%s fpath=%s idx=%i\n", fname_lwr.c_str(), fpath.c_str(), id);
									trie.update(fname_lwr.c_str(), fname_lwr.length(), id);
								}
								else
								{
									tmp_props[it->second].m_fpath.push_back(fpath);
									//dbg_printf("update: fname=%s fpath=%s idx=%i\n", fname.c_str(), fpath.c_str(), it->second);
								}
							 });*/
				}
				props = tmp_props;
			}
			catch (...)
			{
			}

			//printf("keys: %ld\n", trie.num_keys ());
			//printf("size: %ld\n", trie.size ());
		}
	}

	void ParsedFileCache::MakeIndex ()
	{
		makeIndex(m_index.m_trie, m_index.m_props);
	}

}
