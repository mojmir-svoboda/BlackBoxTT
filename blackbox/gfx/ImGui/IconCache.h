#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include <memory>
#include <blackbox/gfx/IconId.h>
#include <loki/AssocVector.h>
#include <imgui/imgui.h>
struct ID3D11ShaderResourceView;

namespace bb {

	uint32_t const c_maxIconTextureSize = 2048;

	struct IconSlab
	{
		uint16_t m_x { 0 }; // icon x size
		uint16_t m_y { 0 }; // icon y size
		uint16_t m_nx { 0 }; // number of icons in x dir
		uint16_t m_ny { 0 }; // number of icons in y dir
		uint8_t m_bits { 0 };
		uint16_t m_end { 0 };
		uint16_t m_updated { 0 };
		std::unique_ptr<uint8_t[]> m_buffer;

		ID3D11ShaderResourceView * m_view { nullptr };
		std::vector<bbstring> m_names;

		~IconSlab ();

		bool Init (uint32_t x, uint32_t y, uint32_t nx, uint32_t ny, uint32_t bits);
		bool AddIconToSlab (bbstring const & name, BITMAP const & b, uint8_t * buff, size_t buffsz, IconId & id);
		void Update ();
		bool IsFull () const { return m_end >= m_nx * m_ny; }
		bool Get (uint32_t index, ImTextureID & texid, ImVec2 & uv0, ImVec2 & uv1) const;
		bool Find (bbstring const & name, IconId & id) const;
	};

	struct IconSlabs
	{
		std::vector<std::unique_ptr<IconSlab>>  m_slabs;

		bool AddIconToSlab (bbstring const & name, BITMAP const & b, uint8_t * buff, size_t buffsz, IconId & id);
		void Update ();
		bool Find (bbstring const & name, IconId & id) const;
	};


	struct IconCache
	{
		using slabs_t = Loki::AssocVector<unsigned, IconSlabs *>;
		slabs_t m_slabs;

		bool Add (bbstring const & name, HICON ico, IconId & id);
		bool Find (bbstring const & name, IconId & id) const;
		bool GetSlab (IconId id, IconSlab const * & icon_slab) const;
		void Update ();
	};
}
