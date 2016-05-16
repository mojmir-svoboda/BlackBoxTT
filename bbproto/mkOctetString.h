#pragma once
#include <OCTET_STRING.h>

namespace asn1 {

	inline OCTET_STRING_t mkOctetStringRaw (char const * raw_str, size_t ln)
	{
		OCTET_STRING_t ostr;
		ostr.buf = const_cast<unsigned char *>(reinterpret_cast<unsigned char const *>(raw_str));
		ostr.size = ln;
		return ostr;
	}

	inline OCTET_STRING_t mkOctetString (char const * raw_str)
	{
		OCTET_STRING_t ostr;
		ostr.buf = const_cast<unsigned char *>(reinterpret_cast<unsigned char const *>(raw_str));
		ostr.size = strlen(raw_str);
		return ostr;
	}
}

