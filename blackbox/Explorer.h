#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include <ShlObj.h>
#include <vector>
#include "Gfx/IconId.h"
#include <loki/AssocVector.h>
#include "ExplorerItem.h"
#include "ExplorerConfig.h"

namespace bb {

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
		ExplorerConfig m_config;
		LPMALLOC m_allocator;
		IShellFolder * m_shell;
		std::vector<ExplorerItem> m_controlPanel;
		std::vector<ExplorerItem> m_startMenu;
		Loki::AssocVector<bbstring, KnwnFldr> m_knownFolders;

		Explorer ();
		bool Init (ExplorerConfig const & cfg);
		void Update ();
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

		void HideExplorer ();
		void ShowExplorer ();
		bool IsExplorerVisible () const;
		LPITEMIDLIST GetItemId (bbstring const & path);

		void OnClickedAt (PIDLIST_ABSOLUTE pidl);
		bool IsFolder (PIDLIST_ABSOLUTE pidl) const;
	};
}
