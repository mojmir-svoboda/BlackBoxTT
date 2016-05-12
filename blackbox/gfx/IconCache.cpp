#pragma once
#include "IconCache.h"
#include "utils_gdi.h"
#include <imgui/imgui.h>
#include <blackbox/BlackBox.h>

namespace bb {

	void IconSlab::Update ()
	{
		if (m_dirty)
		{
			m_dirty = false;
			uint32_t const x = m_x * m_nx;
			uint32_t const y = m_y * m_ny;
			if (m_view == nullptr)
			{
				m_view = BlackBox::Instance().GetGfx().MkIconResourceView(x, y, m_bits, m_buffer.get());
			}
			else
				BlackBox::Instance().GetGfx().UpdateIconResourceView(x, y, m_bits, m_buffer.get(), m_view);
		}
	}

	bool IconSlab::Init (uint32_t x, uint32_t y, uint32_t nx, uint32_t ny, uint32_t bits)
	{
		m_x = x;
		m_y = y;
		m_nx = nx;
		m_ny = ny;
		m_end = 0;
		m_bits = bits;
		size_t n = x * nx * y * ny * bits / CHAR_BIT;
		m_buffer.reset(new uint8_t[n]);
		memset(m_buffer.get(), 0, n);
		return true;
	}

	bool IconSlab::AddIconToSlab (bbstring const & name, BITMAP const & b, uint8_t * buff, size_t buffsz, IconId & id)
	{
		if (IsFull())
			return false;

		uint32_t const idx = m_end++;
		id.m_index = idx;
		m_names.push_back(name);
		uint32_t const bgn = idx * m_nx;
		uint32_t const start = idx * m_x;
		uint32_t const bits = m_bits / CHAR_BIT;
		uint32_t const x_dim = m_x * m_nx * bits;
		uint32_t const yend = b.bmHeight;

		// bmp cpy
		for (uint32_t y = 0; y < yend; ++y)
		{
			size_t const dst_offs = start + y * x_dim;
			void * dst = m_buffer.get() + dst_offs;
			size_t const src_offs = y * m_x * bits;
			void * src = buff + src_offs;
			memcpy(dst, src, m_x * bits);
		}

		m_dirty = true;
		return true;
	}

	bool IconSlab::Get (uint32_t index, ImTextureID & texid, ImVec2 & uv0, ImVec2 & uv1) const
	{
		if (index < m_nx * m_ny)
		{
			uint32_t const sz = static_cast<float>(m_x);
			uint32_t const x = index % m_nx;
			uint32_t const y = index / m_nx;
			uint32_t const b = m_bits / CHAR_BIT;
			float const u0x = x * m_x * b;
			float const u0y = y * m_y;
			float const u1x = u0x + sz;
			float const u1y = u0y + sz;
			float const szx = m_x * m_nx;
			float const szy = m_y * m_ny;
			float const u0 = u0x / szx;
			float const v0 = u0y / szy;
			float const u1 = u1x / szx;
			float const v1 = u1y / szy;
			texid = static_cast<void *>(m_view);
			uv0 = ImVec2(u0, v0);
			uv1 = ImVec2(u1, v1);
			return true;
		}
		return false;
	}

	bool IconSlabs::AddIconToSlab (bbstring const & name, BITMAP const & b, uint8_t * buff, size_t buffsz, IconId & id)
	{
		bool placed = false;
		for (size_t s = 0, se = m_slabs.size(); s < se; ++s)
		{
			IconSlab & slab = m_slabs[s];
			if (slab.IsFull())
				continue;
			else
			{
				if (placed = slab.AddIconToSlab(name, b, buff, buffsz, id))
				{
					id.m_slab = s;
					return true;
				}
			}
		}
		if (!placed)
		{
			size_t const s = m_slabs.size();
			m_slabs.push_back(IconSlab());
			IconSlab & slab = m_slabs.back();
/////////////////////////////////////////////////////////////////////////////////////////////
			if (s)
				; // warning is to remind me
			uint32_t const nx = 1;//c_maxIconTextureSize / b.bmWidth;
			uint32_t const ny = 1;//c_maxIconTextureSize / b.bmHeight;
			//uint32_t const nx = c_maxIconTextureSize / b.bmWidth;
			//uint32_t const ny = c_maxIconTextureSize / b.bmHeight;
			slab.Init(b.bmWidth, b.bmHeight, nx, ny, b.bmBitsPixel);
			id.m_slab = s;
			return slab.AddIconToSlab(name, b, buff, buffsz, id);
		}
		return false;
	}

	void IconSlabs::Update ()
	{
		for (auto & it : m_slabs)
			it.Update();
	}

	bool IconCache::Add (bbstring const & name, HICON ico, IconId & id)
	{
		ICONINFO iconInfo;
		BITMAP b;
		if (getIconBmpDesc(ico, iconInfo, b))
		{
			size_t const buffsz = b.bmWidth * b.bmHeight * b.bmBitsPixel / 8;
			uint8_t * const buff = reinterpret_cast<uint8_t *>(alloca(buffsz * sizeof(uint8_t)));

			if (size_t const sz = iconToBuffer(iconInfo, b, buff, buffsz))
			{
				id.m_size = b.bmWidth;
				IconSlabs * & slabs = m_slabs[b.bmWidth];
				if (slabs == nullptr)
				{
					IconSlabs * const new_slabs = new IconSlabs;
					if (!new_slabs)
						return false;
					slabs = new_slabs;
				}
				return slabs->AddIconToSlab(name, b, buff, buffsz, id);
			}
		}
		return false;
	}

	bool IconCache::GetSlab (IconId id, IconSlab const * & icon_slab) const
	{
		auto it = m_slabs.find(id.m_size);
		if (it != m_slabs.end())
		{
			IconSlabs const * const slabs = it->second;
			if (id.m_index < slabs->m_slabs.size())
			{
				IconSlab const & slab = slabs->m_slabs[id.m_slab];
				icon_slab = &slab;
				return true;
			}
		}
		icon_slab = nullptr;
		return false;
	}

	void IconCache::Update ()
	{
		for (auto it : m_slabs)
			it.second->Update();
	}
}
