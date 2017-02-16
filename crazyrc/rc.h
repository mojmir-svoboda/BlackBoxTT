#pragma once
#include "unicode.h"
#if defined (libcrazyrc_EXPORTS)
#	define CRAZYRC_API __declspec(dllexport)
#else
#	define CRAZYRC_API __declspec(dllimport)
#endif

namespace rc {

	CRAZYRC_API bool init ();

	CRAZYRC_API bool readString (wchar_t const * fname, wchar_t const * key, wchar_t const * defaultString, wchar_t * result, size_t resultsz);
	CRAZYRC_API bool readConstString (wchar_t const * fname, wchar_t const * key, wchar_t const * defaultString, wchar_t const * & result);
	CRAZYRC_API bool readBool (wchar_t const * fname, wchar_t const * key, bool defaultval, bool & result);
	CRAZYRC_API bool readInt (wchar_t const * fname, wchar_t const * key, int defaultval, int & result);

	CRAZYRC_API bool readString (tstring const & fname, tstring const & key, tstring const & defaultString, tstring & result);
	CRAZYRC_API bool readBool (tstring const & fname, tstring const & key, bool defaultval, bool & result);

	CRAZYRC_API bool done ();
}
