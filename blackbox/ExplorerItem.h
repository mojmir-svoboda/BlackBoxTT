#pragma once
#include <common.h>
#include <ShlObj.h>
#include "Gfx/IconId.h"

namespace bb {

	struct Pidl
	{
		LPITEMIDLIST m_pidl;

		Pidl () : m_pidl(0) { }
		Pidl (LPITEMIDLIST id) : m_pidl(ILClone(id)) { }
		Pidl (Pidl const & rhs) : m_pidl(rhs.m_pidl) { }
		Pidl & operator= (Pidl const & rhs)
		{
			if (this != &rhs)
			{
				m_pidl = ILClone(rhs.m_pidl);
			}
			return *this;
		}
		Pidl (Pidl && rhs) : m_pidl(rhs.m_pidl) { rhs.m_pidl = 0; }
		~Pidl ()
		{
			if (m_pidl)
			{
				ILFree(m_pidl);
			}
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

		ExplorerItem (LPITEMIDLIST pidl, bbstring const & name, bbstring const & ico, int icoidx, IconId sml_id, IconId lrg_id)
			: m_pidl(pidl), m_name(name), m_icoPath(ico), m_icoIdx(icoidx), m_icoSmall(sml_id), m_icoLarge(lrg_id)/*, m_icon(nullptr)*/
		{ }

		ExplorerItem (ExplorerItem && rhs)
			: m_pidl(std::move(rhs.m_pidl)), m_name(std::move(rhs.m_name)), m_icoPath(std::move(rhs.m_icoPath)), m_icoIdx(rhs.m_icoIdx), m_icoSmall(rhs.m_icoSmall), m_icoLarge(rhs.m_icoLarge) /*, m_icon(rhs.m_icon)*/
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

		ExplorerItem () { }

		~ExplorerItem ()
		{
		}
	};
}
