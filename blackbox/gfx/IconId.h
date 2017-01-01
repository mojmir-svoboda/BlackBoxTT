#pragma once

namespace bb {

	struct IconId
	{
		unsigned m_size : 11; // 2048 max icon size
		uint8_t m_slab : 5;		// 32 slabs
		uint16_t m_index;			// with 64k entries each

		IconId () : m_size(0), m_slab(0), m_index(0) { }
		IconId (uint32_t sz, uint32_t s, uint32_t i) : m_size(sz), m_slab(s), m_index(i) { }
		bool IsValid () const { return m_size > 0; }
	};
}
