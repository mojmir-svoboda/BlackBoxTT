#pragma once
#include "IconCache.h"
#include <blackbox/gfx/utils_gdi.h>
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/shared/DX11.h>
#include <blackbox/common.h>

namespace bb {
namespace shared {

	IconSlab::~IconSlab ()
	{
		if (m_view)
		{
			m_view->Release();
			m_view = nullptr;
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

	bool IconSlab::Find (bbstring const & name, IconId & id) const
	{
		for (size_t i = 0, ie = m_names.size(); i < ie; ++i)
			if (m_names[i] == name)
			{
				id.m_index = i;
				return true;
			}
		return false;
	}

	bool IconSlab::AddIconToSlab (bbstring const & name, BITMAP const & b, uint8_t * buff, size_t buffsz, IconId & id)
	{
		if (IsFull())
			return false;

		uint32_t const idx = m_end++;
		Assert(idx <= IconId(~0U, ~0U, ~0U).m_index);
		id.m_index = idx;
		m_names.push_back(name);
		uint32_t const bytes = m_bits / CHAR_BIT;
		uint32_t const x_dim = m_x * m_nx * bytes;
		uint32_t const bgn = idx * m_x * bytes;
		uint32_t const yend = b.bmHeight;

		// bmp cpy
		for (uint32_t y = 0; y < yend; ++y)
		{
			size_t const dst_offs = bgn + y * x_dim;
			void * dst = m_buffer.get() + dst_offs;
			size_t const src_offs = y * m_x * bytes;
			void * src = buff + src_offs;
			memcpy(dst, src, m_x * bytes);
		}

		return true;
	}

	bool IconSlab::Get (uint32_t index, void * & texid, float & x0, float & y0, float & x1, float & y1) const
	{
		if (index < m_nx * m_ny)
		{
			uint32_t const sz = static_cast<float>(m_x);
			uint32_t const x = index % m_nx;
			uint32_t const y = index / m_nx;
			float const u0x = x * m_x;
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
			x0 = u0;
			y0 = v0;
			x1 = u1;
			y1 = v1;
			return true;
		}
		return false;
	}

	bool IconSlabs::AddIconToSlab (bbstring const & name, BITMAP const & b, uint8_t * buff, size_t buffsz, IconId & id)
	{
		bool placed = false;
		for (size_t s = 0, se = m_slabs.size(); s < se; ++s)
		{
			IconSlab & slab = *m_slabs[s];
			if (slab.IsFull())
				continue;
			else
			{
				if (placed = slab.AddIconToSlab(name, b, buff, buffsz, id))
				{
					Assert(s <= IconId(~0U, ~0U, ~0U).m_slab);
					id.m_slab = s;
					return true;
				}
			}
		}
		if (!placed)
		{
			size_t const s = m_slabs.size();
			std::unique_ptr<IconSlab> new_slab(new IconSlab);
			m_slabs.push_back(std::move(new_slab));
			IconSlab & slab = *m_slabs.back();

			//uint32_t const nx = 1; // to test slabs (1x ico per slab)
			//uint32_t const ny = 1;
			uint32_t const nx = c_maxIconTextureSize / b.bmWidth;
			uint32_t const ny = c_maxIconTextureSize / b.bmHeight;
			slab.Init(b.bmWidth, b.bmHeight, nx, ny, b.bmBitsPixel);
			if (slab.AddIconToSlab(name, b, buff, buffsz, id))
			{
				Assert(s <= IconId(~0U, ~0U, ~0U).m_slab);
				id.m_slab = s;
				return true;
			}
		}
		return false;
	}

	bool IconSlabs::Find (bbstring const & name, IconId & id) const
	{
		for (size_t s = 0, se = m_slabs.size(); s < se; ++s)
			if (m_slabs[s]->Find(name, id))
			{
				Assert(s <= IconId(~0U, ~0U, ~0U).m_slab);
				id.m_slab = s;
				return true;
			}
		return false;
	}

	bool IconCache::Find (bbstring const & name, IconId & id) const
	{
		for (auto & it : m_slabs)
			if (it.second->Find(name, id))
			{
				id.m_size = it.first;
				return true;
			}
		return false;
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
			if (id.m_slab < slabs->m_slabs.size())
			{
				IconSlab const & slab = *slabs->m_slabs[id.m_slab];
				icon_slab = &slab;
				return true;
			}
		}
		icon_slab = nullptr;
		return false;
	}
}}
