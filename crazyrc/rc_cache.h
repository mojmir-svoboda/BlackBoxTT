#pragma once
#include <unordered_map>
#include <boost/variant/get.hpp>
#include "unicode.h"
#include "rc_types.h"
#include "rc_index.h"

namespace rc {

	struct ParsedFileRecord
	{
		file_type m_parsedFile;
		typedef std::unordered_map<tstring, value_type> cache_t;

	};

	struct ParsedFileCache
	{
		typedef std::unordered_map<tstring, ParsedFileRecord> cache_t;
		cache_t m_cache;
		index::Index m_index;

		bool Add (tstring const & filename, ParsedFileRecord const & rec)
		{
			m_cache.insert(std::make_pair(filename, rec));
			return true;
		}
		bool Add (wchar_t const * filename, ParsedFileRecord const & rec)
		{
			m_cache.insert(std::make_pair(tstring(filename), rec));
			return true;
		}

		template <class T>
		bool Lookup (tstring const & fname, tstring const & key, T & result)
		{
			std::vector<std::pair<tstring, size_t>> candidates;
			if (m_index.Lookup(fname, key, candidates))
			{
				tstring const & file = candidates[0].first;
				size_t const line = candidates[0].second;
				cache_t::iterator it = m_cache.find(file);
				if (it != m_cache.end())
				{
					if (it->second.m_parsedFile[line].which() == e_line_pair)
					{
						pair_type const & p = boost::get<pair_type const &>(it->second.m_parsedFile[line]);

						if (p.second) // optional check
						{
							value_variant_type const & val = *p.second;
							return GetAs<T>(val, result);
						}
					}
				}
				return true;
			}
			return false;
		}

		template<class T>
		bool GetAs (value_variant_type const & val, T & result);

		template<>
		bool GetAs (value_variant_type const & val, tstring & result)
		{
			//BB_ASSERT(val.which() == e_val_string, "type mismatch. getting string, but value is not. value type=%i", val.which());
			if (val.which() == e_val_string)
			{
				result = boost::get<decltype(result) const &>(val);
				return true;
			}
			return false;
		}
		template<>
		bool GetAs (value_variant_type const & val, int & result)
		{
			//BB_ASSERT(val.which() == e_val_string, "type mismatch. getting string, but value is not. value type=%i", val.which());
			if (val.which() == e_val_int)
			{
				result = boost::get<decltype(result) const &>(val);
				return true;
			}
			return false;
		}
		template<>
		bool GetAs (value_variant_type const & val, bool & result)
		{
			//BB_ASSERT(val.which() == e_val_string, "type mismatch. getting string, but value is not. value type=%i", val.which());
			if (val.which() == e_val_bool)
			{
				result = boost::get<decltype(result) const &>(val);
				return true;
			}
			return false;
		}


		bool IsFileCached (tstring const & filename) const
		{
			return m_cache.find(filename) != m_cache.end();
		}

		void MakeIndex ();

		void Remove (tstring const & filename)
		{
		}
	};

	ParsedFileCache const & getParsedFileCacheConst ();
	ParsedFileCache & getParsedFileCache ();
}

