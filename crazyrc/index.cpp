#include "index.h"
#include "config.h"
#include "search.h"
#include <fstream>
#include <blackbox/BBApi.h>
#include "serialize.h"
#include <map>

namespace bb { namespace search {

struct icasecompare : std::binary_function<tstring, tstring, bool> {
    bool operator() (tstring const & lhs, tstring const & rhs) const {
        return _tcsicmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

void makeIndex (trie_t & trie, props_t & props, Config const & cfg, tstring const & fname2)
{
	try
	{
		props_t tmp_props;
		tmp_props.reserve(1024);
		std::map<tstring, int, icasecompare> tmp_propmap;
		for (SearchLocationInfo const & info : cfg.m_locations)
		{
			SearchDirectory(
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
					 });
		}
		props = tmp_props;
	}
	catch (std::regex_error const & e)
	{
		dbg_printf("Exception caught: %s", e.what());
	}

	//printf("keys: %ld\n", trie.num_keys ());
	//printf("size: %ld\n", trie.size ());
}

bool saveIndex (trie_t const & t, tstring const & fpath)
{
	if (t.save(fpath.c_str()) != 0)
	{
		// err
		return false;
	}
	return true;
}

bool loadIndex (trie_t & t, tstring const & fpath)
{
	return t.open(fpath.c_str()) == 0;
}

bool loadProps (props_t & t, tstring const & fpath)
{
	std::fstream f(fpath.c_str(), std::ios::in | std::ios::binary);
	if (!f)
		return false;

	unsigned v = 0;
	if (read(f, v).fail())
	{
		f.close();
		return false;
	}
	if (v != version11)
	{
		f.close();
		return false;
	}

	if (read(f, t).fail())
	{
		f.close();
		return false;
	}

	f.close();
	return true;
}

bool saveProps (props_t const & t, tstring const & fpath)
{
	std::fstream f(fpath.c_str(), std::ios::out | std::ios::binary);
	if (!f)
		return false;

	if (write(f, version11).fail())
	{
		f.close();
		return false;
	}
	if (write(f, t).fail())
	{
		f.close();
		return false;
	}
	f.close();
	return true;
}

bool loadForget (forget_t & t, tstring const & fpath)
{
	t.reserve(512);
	std::ifstream f(fpath.c_str());
	if (f.fail())
		return false;

	std::string line;
	while (std::getline(f, line))
	{
		if (!line.empty())
			t.push_back(line);
	}
	f.close();
	return true;
}

bool saveForget (forget_t const & t, tstring const & fpath)
{
	std::ofstream f(fpath.c_str());
	if (f.fail())
		return false;
	for (tstring const & s : t)
		f << s << std::endl;
	f.close();
	return true;
}

}}
