#pragma once
#include <codecvt>
#include <utility>
#include <platform_win.h>

namespace bb {

	// utf8 -> utf16
	inline size_t codecvt_utf8_utf16_dst_size (char const * src, size_t srcln)
	{
		size_t const size_needed = MultiByteToWideChar(CP_UTF8, 0, src, srcln, NULL, 0);
		return size_needed + 1;
	}
	inline size_t codecvt_utf8_utf16 (char const * src, size_t srcln, wchar_t * dst, size_t dstsz)
	{
		size_t size_needed = MultiByteToWideChar(CP_UTF8, 0, src, srcln, NULL, 0);
		if (size_needed > dstsz)
			size_needed = dstsz;
		int const n = MultiByteToWideChar(CP_UTF8, 0, src, srcln, dst, size_needed);
		if (n == dstsz)
			dst[n - 1] = NULL;
		else
			dst[n] = NULL;
		return size_needed;
	}
	inline size_t codecvt_utf8_utf16 (std::string const & src, wchar_t * dst, size_t dstsz)
	{
		return codecvt_utf8_utf16(src.c_str(), src.length(), dst, dstsz);
	}
	inline size_t codecvt_utf8_utf16 (std::string const & src, std::wstring & dst)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		dst = convert.from_bytes(src);
		return true;
	}

	// utf16 -> utf8
	inline size_t codecvt_utf16_utf8_dst_size (wchar_t const * src, size_t srcln)
	{
		size_t const size_needed = WideCharToMultiByte(CP_UTF8, 0, src, srcln, NULL, 0, NULL, NULL);
		return size_needed + 1;
	}
	inline size_t codecvt_utf16_utf8 (wchar_t const * src, size_t srcln, char * dst, size_t dstsz)
	{
		size_t size_needed = WideCharToMultiByte(CP_UTF8, 0, src, srcln, NULL, 0, NULL, NULL);
		if (size_needed > dstsz)
			size_needed = dstsz;
		int const n = WideCharToMultiByte(CP_UTF8, 0, src, srcln, dst, size_needed, NULL, NULL);
		if (n == dstsz)
			dst[n - 1] = NULL;
		else
			dst[n] = NULL;
		return size_needed;
	}
	inline size_t codecvt_utf16_utf8 (std::wstring const & src, char * dst, size_t dstsz)
	{
		return codecvt_utf16_utf8(src.c_str(), src.length(), dst, dstsz);
	}
	inline bool codecvt_utf16_utf8 (std::wstring const & src, std::string & dst)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		dst = convert.to_bytes(src);
		return true;
	}
}
