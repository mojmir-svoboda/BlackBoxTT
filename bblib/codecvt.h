#pragma once
#include <codecvt>
#include <utility>
#include <platform_win.h>

namespace bb {

	// utf8 -> utf16
	inline bool codecvt_utf8_utf16 (std::string const & src, wchar_t * dst, size_t dstsz)
	{
		size_t const size = src.length();
		size_t size_needed = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), size, NULL, 0);
		if (size_needed > dstsz)
			size_needed = dstsz;
		int const n = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), size, dst, size_needed);
		if (n == dstsz)
			dst[n - 1] = NULL;
		else
			dst[n] = NULL;
		return size_needed;
	}
	inline bool codecvt_utf8_utf16 (std::string const & src, std::wstring & dst)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		dst = convert.from_bytes(src);
		return true;
	}

	// utf16 -> utf8
	inline bool codecvt_utf16_utf8 (std::wstring const & src, char * dst, size_t dstsz)
	{
		size_t const w_size = src.length();
		size_t size_needed = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), w_size, NULL, 0, NULL, NULL);
		if (size_needed > dstsz)
			size_needed = dstsz;
		int const n = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), w_size, dst, size_needed, NULL, NULL);
		if (n == dstsz)
			dst[n - 1] = NULL;
		else
			dst[n] = NULL;
		return size_needed;
	}
	inline bool codecvt_utf16_utf8 (std::wstring const & src, std::string & dst)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		dst = convert.to_bytes(src);
		return true;
	}
}
