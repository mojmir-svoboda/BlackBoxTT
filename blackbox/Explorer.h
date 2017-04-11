#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include <ShlObj.h>
#include <vector>
#include "Gfx/IconId.h"

namespace bb {

	struct ExplorerConfig
	{
		bool m_hide;
		bool m_hideTray;
	};

	struct Pidl
	{
		LPITEMIDLIST m_pidl;

		Pidl () : m_pidl(0) { }
		Pidl (LPITEMIDLIST id) : m_pidl(ILClone(id)) { }
		Pidl (Pidl && rhs) : m_pidl(rhs.m_pidl) { rhs.m_pidl = 0; }
		~Pidl ()
		{
			if (m_pidl)
			{
				ILFree(m_pidl);
			}
		}

		void Deallocate ();
	};

	inline LPITEMIDLIST clone (LPITEMIDLIST src)
	{
	}

	struct ExplorerItem
	{
		Pidl m_pidl;
		bbstring m_name;
		bbstring m_icoPath;
		int m_icoIdx;
		//HICON m_icon; // do i need it? maybe in async case...
		IconId m_icoSmall;
		IconId m_icoLarge;

		ExplorerItem (LPITEMIDLIST pidl, bbstring const & name, bbstring const & ico, int icoidx, IconId sml_id, IconId lrg_id)
			: m_pidl(pidl), m_name(name), m_icoPath(ico), m_icoIdx(icoidx), m_icoSmall(sml_id), m_icoLarge(lrg_id)/*, m_icon(nullptr)*/
		{ }

		ExplorerItem (ExplorerItem && rhs)
			: m_pidl(std::move(rhs.m_pidl)), m_name(std::move(rhs.m_name)), m_icoPath(std::move(rhs.m_icoPath)), m_icoIdx(rhs.m_icoIdx), m_icoSmall(rhs.m_icoSmall), m_icoLarge(rhs.m_icoLarge) /*, m_icon(rhs.m_icon)*/
		{ }

		~ExplorerItem ()
		{
		}
	};

	struct Explorer
	{
		LPMALLOC m_allocator;
		IShellFolder * m_shell;
		std::vector<ExplorerItem> m_controlPanel;
		std::vector<ExplorerItem> m_startMenu;

		Explorer ();
		bool Init ();
		bool InitControlPanel ();
		bool InitStartMenu ();
		bool KnownFolderEnumerate (REFKNOWNFOLDERID rfid, std::vector<ExplorerItem> & result);
		bool Done ();

		void HideExplorer (ExplorerConfig const & cfg);
		void ShowExplorer ();
		LPITEMIDLIST GetItemId (bbstring const & path);

		void OnClickedAt (Pidl const & pidl);
	};
}
