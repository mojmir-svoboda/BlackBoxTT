#include "rc.h"
#include "rc_cache.h"
#include "rc_parser.h"
#include "rgb_txt_parser.h"

namespace rc {


	bool init ()
	{
		rgb_txt::readColorFileToGlobalTable(TEXT("c:\\devel\\bbr\\rc\\config\\rgb.txt"));
		return true;
	}

	bool readString (tstring const & fname, tstring const & key, tstring const & defaultString, tstring & result)
	{
		if (!rc::getParsedFileCacheConst().IsFileCached(fname))
		{
			bool const parsed = rc::parseFile(fname);
			if (!parsed)
			{
				result = defaultString;
				return false;
			}
		}

		// query cache
		if (!rc::getParsedFileCache().Lookup(fname, key, result))
			result = defaultString;
		return true;
	}

	bool readString (wchar_t const * fname, wchar_t const * key, wchar_t const * defaultString, wchar_t * result, size_t resultsz)
	{
		if (!rc::getParsedFileCacheConst().IsFileCached(tstring(fname)))
		{
			bool const parsed = rc::parseFile(fname);
			if (!parsed)
			{
				_tcsncpy(result, defaultString, resultsz);
				return false;
			}
		}

		// query cache
		tstring res;
		if (!rc::getParsedFileCache().Lookup(tstring(fname), tstring(key), res))
			_tcsncpy(result, defaultString, resultsz);
		else
			_tcsncpy(result, res.c_str(), res.length());
		return true;
	}

	bool readInt (wchar_t const * fname, wchar_t const * key, int default_value, int & value)
	{
		value = default_value;
		if (!rc::getParsedFileCacheConst().IsFileCached(tstring(fname)))
			if (!rc::parseFile(fname))
				return false;
		return rc::getParsedFileCache().Lookup(tstring(fname), tstring(key), value); // query cache
	}

	bool readBool (wchar_t const * fname, wchar_t const * key, bool default_value, bool & value)
	{
		value = default_value;
		if (!rc::getParsedFileCacheConst().IsFileCached(tstring(fname)))
			if (!rc::parseFile(fname))
				return false;
		return rc::getParsedFileCache().Lookup(tstring(fname), tstring(key), value); // query cache
	}

}
