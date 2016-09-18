#pragma once
#include <functional>
#include <vector>
#include "unicode.h"
#include "rc_types.h"
#include <3rd_party/cedar/cedar.h>

namespace rc { namespace index {

struct Config { };

struct Props
{
	tstring m_key;
	std::vector<size_t> m_indices;
	std::vector<tstring> m_rcfiles; /// rc files containing

	Props (tstring const & key, size_t idx, tstring const & rcfile)
		: m_key(key)
		, m_rcfiles()
	{
		m_indices.push_back(idx);
		m_rcfiles.push_back(rcfile);
	}
	Props () { }
};

typedef std::vector<Props> props_t; // vecor
typedef cedar::da<int> trie_t;

bool searchIndex (trie_t & t, tstring const & str, std::function<void(tstring const &, tstring const &)> on_match);

struct Index
{
	trie_t m_trie;
	props_t m_props;

	bool IsLoaded () const { return m_props.size() > 0; }

	bool Find (tstring const & what, std::vector<std::pair<tstring, size_t>> & results, size_t max_results = 128)
	{
		results.clear();
		results.reserve(max_results);
		return SearchIndex(what, max_results,
			[&results](tstring const & rcfile, size_t idx)
				{
					results.push_back(std::make_pair(rcfile, idx));
				});
	}

	/*bool Suggest (tstring const & what, std::vector<tstring> & keywords, std::vector<tstring> & results, size_t max_results = 128)
	{
		results.clear();
		results.reserve(max_results);
		keywords.clear();
		keywords.reserve(max_results);
		return SearchIndex(what, max_results,
				[&results, &keywords] (tstring const & rcfile, size_t index)
				{
					keywords.push_back(fname);
					results.push_back(fpath);
				});
	}*/

	bool Lookup(tstring const & fname, tstring const & key, std::vector<std::pair<tstring, size_t>> & results)
	{
		bool const res = Find(key, results, 128);
		if (res && results.size())
		{
			//result = results[0].first; //@TODO
			return true;
		}
		return false;
	}

	void Clear ()
	{
		m_trie.clear();
		m_props.clear();
	}

protected:
	bool SearchIndex (tstring const & str, size_t max_results, std::function<void(tstring const &, size_t)> on_match)
	{
		if (!IsLoaded())
			return false;

		trie_t::result_triple_type * const result_triple = static_cast<trie_t::result_triple_type *>(alloca(max_results * sizeof(trie_t::result_triple_type)));
		TCHAR suffix[1024];
		if (const size_t n = m_trie.commonPrefixPredict(str.c_str(), result_triple, max_results))
		{
			for (size_t i = 0; i < n && i < max_results; ++i)
			{
				m_trie.suffix(suffix, result_triple[i].length, result_triple[i].id);
				for (size_t j = 0, je = m_props[result_triple[i].value].m_rcfiles.size(); j < je; ++j)
				//for (tstring const & s : m_props[result_triple[i].value].m_rcfiles)
				{
					size_t index = m_props[result_triple[i].value].m_indices[j];
					//if (m_)
					//tstring const & val = 
					wprintf(TEXT("%s: prefix found, [%d/%d] file_index=%i\n"), str.c_str(), static_cast<int>(n), static_cast<int>(i), static_cast<int>(index));
					on_match(m_props[result_triple[i].value].m_rcfiles[j], index);
				}
			}
			return true;
		}
		return false;
	}
};

}}

