#pragma once
#include <string>

// important notice:
// * everything in std::string is utf8
// * everything in std::wstring is unicode

using bbstring = std::wstring;

// inline void unquote (bbstring & dst, bbstring const & src)
// {
// 	size_t const l = src.length();
// 	if (l >= 2 && (src[0] == L'\"' || src[0] == L'\'') && src[l - 1] == src[0])
// 		dst = src.substr(1, l - 2);
// }

