#pragma once
#include "TrayConfig.h"
#include "platform_win.h"
#include "hooks/trayhook.h"
#include <vector>

namespace bb {

	struct TrayItem
	{
		HWND m_hwnd;
	};

	struct Tray
	{
		std::vector<TrayItem> m_items;
		TrayData m_data;

		bool Init (TrayConfig & cfg);
		bool Done ();

		void EnumTray ();
		void UpdateFromTrayHook ();
	};
}

