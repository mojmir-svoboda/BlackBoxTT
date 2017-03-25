#pragma once
#include <private/qsimd_p.h>
#include <private/qutfcodec_p.h>

//from Qt
static inline bool simdDecodeAscii (ushort *& dst, const uchar *& nextAscii, const uchar *& src, const uchar * end)
{
	// do sixteen characters at a time
	for ( ; end - src >= 16; src += 16, dst += 16) {
		__m128i data = _mm_loadu_si128((__m128i*)src);

#ifdef __AVX2__
		const int BitSpacing = 2;
		// load and zero extend to an YMM register
		const __m256i extended = _mm256_cvtepu8_epi16(data);

		uint n = _mm256_movemask_epi8(extended);
		if (!n) {
			// store
			_mm256_storeu_si256((__m256i*)dst, extended);
			continue;
		}
#else
		const int BitSpacing = 1;

		// check if everything is ASCII
		// movemask extracts the high bit of every byte, so n is non-zero if something isn't ASCII
		uint n = _mm_movemask_epi8(data);
		if (!n) {
			// unpack
			_mm_storeu_si128((__m128i*)dst, _mm_unpacklo_epi8(data, _mm_setzero_si128()));
			_mm_storeu_si128(1+(__m128i*)dst, _mm_unpackhi_epi8(data, _mm_setzero_si128()));
			continue;
		}
#endif

		// copy the front part that is still ASCII
		while (!(n & 1)) {
			*dst++ = *src++;
			n >>= BitSpacing;
		}

		// find the next probable ASCII character
		// we don't want to load 16 bytes again in this loop if we know there are non-ASCII
		// characters still coming
		n = _bit_scan_reverse(n);
		nextAscii = src + (n / BitSpacing) + 1;
		return false;

	}
	return src == end;
}

inline size_t convertToUnicodeFromUtf8 (const char * chars, size_t len, wchar_t * dest, size_t dest_sz)
{
	// UTF-8 to UTF-16 always needs the exact same number of words or less:
	//	  UTF-8		UTF-16
	//	 1 byte		1 word
	//	 2 bytes	1 word
	//	 3 bytes	1 word
	//	 4 bytes	2 words (one surrogate pair)
	// That is, we'll use the full buffer if the input is US-ASCII (1-byte UTF-8),
	// half the buffer for U+0080-U+07FF text (e.g., Greek, Cyrillic, Arabic) or
	// non-BMP text, and one third of the buffer for U+0800-U+FFFF text (e.g, CJK).
	//
	// The table holds for invalid sequences too: we'll insert one replacement char
	// per invalid byte.
	//QString result(len, Qt::Uninitialized);
	//extern const uchar * utf8bom;
	const uchar utf8bom[] = { 0xef, 0xbb, 0xbf };

	static_assert(sizeof(ushort) == sizeof(wchar_t), "ee");
	ushort * dst = reinterpret_cast<ushort *>(dest);
	const uchar * src = reinterpret_cast<const uchar *>(chars);
	const uchar * end = src + len;

	// attempt to do a full decoding in SIMD
	const uchar * nextAscii = end;
	if (!simdDecodeAscii(dst, nextAscii, src, end))
	{
		// at least one non-ASCII entry
		// check if we failed to decode the UTF-8 BOM; if so, skip it
		if (Q_UNLIKELY(src == reinterpret_cast<const uchar *>(chars))
				&& end - src >= 3
				&& Q_UNLIKELY(src[0] == utf8bom[0] && src[1] == utf8bom[1] && src[2] == utf8bom[2])) {
			src += 3;
		}

		while (src < end) {
			nextAscii = end;
			if (simdDecodeAscii(dst, nextAscii, src, end))
				break;

			do {
				uchar b = *src++;
				int res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(b, dst, src, end);
				if (res < 0) {
					// decoding error
					*dst++ = QChar::ReplacementCharacter;
				}
			} while (src < nextAscii);
		}
	}

	size_t const n = dst - reinterpret_cast<ushort *>(dest);
	return n;
}


