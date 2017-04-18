#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include <ShlObj.h>
#include <vector>
#include "Gfx/IconId.h"
#include <loki/AssocVector.h>
#include "ExplorerItem.h"

namespace bb {

	struct ExplorerConfig
	{
		bool m_hide;
		bool m_hideTray;
	};

// 	inline LPITEMIDLIST clone (LPITEMIDLIST src)
// 	{
// 	}

	struct KnwnFldr
	{
		KNOWNFOLDERID m_rfid;
		//ExplorerItem m_item;
	};

	struct Explorer
	{
		LPMALLOC m_allocator;
		IShellFolder * m_shell;
		std::vector<ExplorerItem> m_controlPanel;
		std::vector<ExplorerItem> m_startMenu;
		Loki::AssocVector<bbstring, KnwnFldr> m_knownFolders;

		Explorer ();
		bool Init ();
		bool InitControlPanel ();
		bool InitStartMenu ();
		void ScanKnownFolders ();
		bool GetExplorerItem (PIDLIST_ABSOLUTE ppidl, ExplorerItem & result);
		bool GetExplorerItem (bbstring const & name, ExplorerItem & result);
		bool FolderEnumerate (PIDLIST_ABSOLUTE ppidl, std::vector<ExplorerItem> & result);
		bool KnownFolderEnumerate (REFKNOWNFOLDERID rfid, std::vector<ExplorerItem> & result);
		bool KnownFolderEnumerate (bbstring const & name, std::vector<ExplorerItem> & result);
		bool KnownFolder (bbstring const & name, std::vector<ExplorerItem> & result);
		bool IsKnownFolder (bbstring const & name) const;
		bool Done ();

		void HideExplorer (ExplorerConfig const & cfg);
		void ShowExplorer ();
		LPITEMIDLIST GetItemId (bbstring const & path);

		void OnClickedAt (Pidl const & pidl);
		bool IsFolder (Pidl const & pidl) const;
	};
}
