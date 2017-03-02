#pragma once
#include <tchar.h>
#include <shlwapi.h>
#include <bblib/wcslcpy.h>

#ifdef UNICODE
# define tmemcpy wmemcpy
# define tmemmove wmemmove
# define tmemset wmemset
# define tmemcmp wmemcmp
# define tmemchr wmemchr
#else
# define tmemcpy memcpy
# define tmemmove memmove
# define tmemset memset
# define tmemcmp memcmp
# define tmemchr memchr
#endif

inline TCHAR * extract_string (TCHAR * dst, TCHAR const * src, int n)
{
    wcslcpy(dst, src, n);
    return dst;
}

inline TCHAR * strcpy_max (TCHAR * dst, TCHAR const * src, int maxlen)
{
    return extract_string(dst, src, maxlen);
}

inline TCHAR * stristr (const TCHAR * aa, const TCHAR * bb)
{
    return StrStrI(aa, bb);
}

inline bool IS_SPC (TCHAR c)
{
	return _istcntrl(c) || _istspace(c);
}

// inline bool startsWith (tstring const & str, tstring const & with)
// {
// 	return str.compare(0, with.length(), with) == 0;
// }


/*char *extract_string(char *dest, const char *src, int n)
{
memcpy(dest, src, n);
dest[n] = 0;
return dest;
}

char *strcpy_max(char *dest, const char *src, int maxlen)
{
int l = (int)strlen(src);
return extract_string(dest, src, l < maxlen ? l : maxlen-1);
}*/

inline TCHAR skip_spc(TCHAR const * & pp)
{
	TCHAR c = 0;
	while (c = *pp, IS_SPC(c) && c)
		++pp;
	return c;
}


inline TCHAR * extract_string (TCHAR * dst, size_t dst_sz, TCHAR const * src, int n)
{
    _tcsncpy_s(dst, n, src, _TRUNCATE);
    return dst;
}

inline void strcpy_max(TCHAR * dest, size_t dest_sz, TCHAR const * src, size_t maxlen)
{
	_tcsncpy_s(dest, dest_sz, src, dest_sz <= maxlen ? _TRUNCATE : maxlen);
}

template <size_t size>
void strcpy_max (TCHAR (&dest)[size], TCHAR const * src, size_t maxlen)
{
	strcpy_max(dest, size, src, maxlen);
}


inline void unquote (TCHAR * dst, size_t dst_sz, TCHAR const * src)
{
	size_t l = _tcslen(src);
	if (l >= 2 && (src[0] == L'\"' || src[0] == L'\'') && src[l - 1] == src[0])
		strcpy_max(dst, dst_sz, src + 1, l - 2);
}

template <size_t buf_sz>
void unquote (TCHAR (&buf)[buf_sz], TCHAR const * src)
{
	unquote(buf, buf_sz, src);
}

// inline void unquote (bbstring & dst, bbstring const & src)
// {
// 	size_t const l = src.length();
// 	if (l >= 2 && (src[0] == L'\"' || src[0] == L'\'') && src[l - 1] == src[0])
// 		dst = src.substr(1, l - 2);
// }


#include <3rd_party/fnv/fnv.h>
inline Fnv32_t calc_hash32 (void const * buff, size_t length_of_buf)
{
    Fnv32_t const hash_val = fnv_32a_buf(buff, length_of_buf, FNV1_32_INIT);
    return hash_val;
}

inline Fnv32_t calc_hash32 (char const * buff)
{
    Fnv32_t const hash_val = fnv_32a_str(buff, FNV1_32_INIT);
    return hash_val;
}
inline Fnv32_t calc_hash32 (wchar_t const * buff)
{
	Fnv32_t const hash_val = fnv_32a_buf(buff, sizeof(wchar_t) * wcslen(buff), FNV1_32_INIT);
	return hash_val;
}

inline Fnv64_t calc_hash64 (const char * buff, size_t length_of_buf)
{
    Fnv64_t const hash_val = fnv_64a_buf(buff, length_of_buf, FNV1_64_INIT);
    return hash_val;
}

inline Fnv64_t calc_hash64 (const char * buff)
{
    Fnv64_t const hash_val = fnv_64a_str(buff, FNV1_64_INIT);
    return hash_val;
}


#if !defined _WIN64
typedef Fnv32_t bb_hash_t;
inline bb_hash_t calc_hash (const char * buff, size_t ln)
{
    return calc_hash32(buff, ln);
}
#else
typedef Fnv64_t bb_hash_t;
inline bb_hash_t calc_hash(const char * buff, size_t ln)
{
    return calc_hash64(buff, ln);
}
#endif

// strlwr a keyword, calculate hash value and length
inline bb_hash_t lower_key_and_calc_hash(TCHAR * dst, size_t dst_max, TCHAR const * src, size_t & dst_len, TCHAR delim)
{
    TCHAR const * ptr = _tcschr(src, delim);
    if (ptr)
    {
        size_t to_cpy = ptr - src;
        _tcsncpy(dst, src, to_cpy);
        dst[to_cpy] = L'\0';
    }
    else
    {
        _tcscpy(dst, src);
    }
    _tcslwr(dst);
    dst_len = _tcslen(dst);
    bb_hash_t const hash = calc_hash(reinterpret_cast<char const *>(dst), dst_len * sizeof(TCHAR));
    return hash;
}

char * replace_arg1(const char * fmt, const char * in);

//BBLIB_EXPORT int replace_string(char *out, int bufsize, int offset, int len, const char *in);
//BBLIB_EXPORT char *extract_string(char *dest, const char *src, int n);
//BBLIB_EXPORT char *strcpy_max(char *dest, const char *src, int maxlen);
//BBLIB_EXPORT char* stristr(const char *aa, const char *bb);
inline int get_string_index (wchar_t const * key, wchar_t const * const * string_array)
{
	const wchar_t * s = nullptr;
	for (int i = 0; nullptr != (s = *string_array); i++, string_array++)
			if (0 == _tcsicmp(key, s))
					return i;
	return -1;
}

