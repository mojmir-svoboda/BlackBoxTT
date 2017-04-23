#pragma once
#include <common.h>
#include <ShlObj.h>
#include "Gfx/IconId.h"

namespace bb {

	struct Pidl
	{
		LPITEMIDLIST m_pidl;

		Pidl () : m_pidl(nullptr) { /*TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "() NOP");*/ }
		Pidl (LPITEMIDLIST pidl) : m_pidl(ILClone(pidl))
		{
			//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "(pidl) pidl=0x%llx m_pidl=0x%llx", pidl, m_pidl);
		}
		Pidl (Pidl const & rhs) : m_pidl(ILClone(rhs.m_pidl))
		{
			//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "(const &) pidl=0x%llx", m_pidl);
		}
		Pidl & operator= (Pidl const & rhs)
		{
			//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "op= (const &) pidl=0x%llx", m_pidl);
			if (this != &rhs)
			{
				m_pidl = ILClone(rhs.m_pidl);
			}
			return *this;
		}
		Pidl & operator= (Pidl && rhs) noexcept
		{
			//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "op= (const &) pidl=0x%llx", m_pidl);
			if (this != &rhs)
			{
				m_pidl = rhs.m_pidl;
				rhs.m_pidl = nullptr;
			}
			return *this;
		}
		Pidl (Pidl && rhs) noexcept
			: m_pidl(rhs.m_pidl)
		{
			//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "(&&) pidl=0x%llx", m_pidl);
			rhs.m_pidl = nullptr;
		}
		~Pidl ()
		{
			if (m_pidl)
			{
				//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "~() free pidl=0x%llx", m_pidl);
				ILFree(m_pidl);
				m_pidl = nullptr;
			}
			//else TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "~() NOP", m_pidl);
		}
	};

	struct ExplorerItem
	{
		Pidl m_pidl;
		bbstring m_name;
		bbstring m_icoPath;
		int m_icoIdx;
		IconId m_icoSmall;
		IconId m_icoLarge;

		ExplorerItem () { }
		~ExplorerItem () { }

		ExplorerItem (LPITEMIDLIST pidl, bbstring const & name, bbstring const & ico, int icoidx, IconId sml_id, IconId lrg_id)
			: m_pidl(pidl), m_name(name), m_icoPath(ico), m_icoIdx(icoidx), m_icoSmall(sml_id), m_icoLarge(lrg_id)
		{ }

		ExplorerItem (ExplorerItem && rhs) noexcept
			: m_pidl(std::move(rhs.m_pidl)), m_name(std::move(rhs.m_name)), m_icoPath(std::move(rhs.m_icoPath)), m_icoIdx(rhs.m_icoIdx), m_icoSmall(rhs.m_icoSmall), m_icoLarge(rhs.m_icoLarge)
		{ }

		ExplorerItem (ExplorerItem const & rhs) noexcept
			: m_pidl(rhs.m_pidl), m_name(rhs.m_name), m_icoPath(rhs.m_icoPath), m_icoIdx(rhs.m_icoIdx), m_icoSmall(rhs.m_icoSmall), m_icoLarge(rhs.m_icoLarge)
		{ }

		ExplorerItem & operator= (ExplorerItem const & rhs)
		{
			if (this != &rhs)
			{
				m_pidl = rhs.m_pidl;
				m_name = rhs.m_name;
				m_icoPath = rhs.m_icoPath;
				m_icoIdx = rhs.m_icoIdx;
				m_icoSmall = rhs.m_icoSmall;
				m_icoLarge = rhs.m_icoLarge;
			}
			return *this;
		}

		ExplorerItem & operator= (ExplorerItem && rhs) noexcept
		{
			if (this != &rhs)
			{
				m_pidl = std::move(rhs.m_pidl);
				m_name = std::move(rhs.m_name);
				m_icoPath = std::move(rhs.m_icoPath);
				m_icoIdx = rhs.m_icoIdx;
				m_icoSmall = rhs.m_icoSmall;
				m_icoLarge = rhs.m_icoLarge;
			}
			return *this;
		}

		bool IsValid () const { return m_pidl.m_pidl != nullptr; }
	};
}

