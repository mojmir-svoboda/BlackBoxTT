#pragma once

namespace bb {

	struct IconId
	{
		unsigned m_size : 11; // 2048 max icon size
		uint8_t m_slab : 5;		// 32 slabs
		uint16_t m_index;			// with 64k entries each

		IconId () : m_size(0), m_slab(0), m_index(0) { }
		bool IsValid () const { return m_size > 0; }
	};
}
