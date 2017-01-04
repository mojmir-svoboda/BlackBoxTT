#pragma once
#include "TasksConfig.h"
#include "TrayConfig.h"
#include "WidgetsConfig.h"
#include "DesktopWallpaperConfig.h"
#include "scheme/SchemeConfig.h"
#include "WorkSpacesConfig.h"
#include "net/ServerConfig.h"
#include "plugin/PluginsConfig.h"
#include "menu/MenuConfig.h"

namespace bb {

	struct OSConfig
	{
		bool m_usingXP;
		bool m_usingVista;
		bool m_usingWin7;
		bool m_usingWin8;
		bool m_usingWin10;

		bool Init ();
	};

	struct DisplayConfig
	{
		int m_monitors;
		int m_x, m_y, m_w, m_h;

		DisplayConfig () : m_monitors(0), m_x(0), m_y(0), m_w(0), m_h(0) { }

		bool Init ();
		bool Update ();
		bool operator== (DisplayConfig const & rhs) const { return m_monitors == rhs.m_monitors && m_x == rhs.m_x && m_y == rhs.m_y && m_w == rhs.m_w && m_h == rhs.m_h; }
		bool operator!= (DisplayConfig const & rhs) const { return !operator==(rhs); }
	};

	struct BlackBoxConfig
	{
		OSConfig m_os;
		DisplayConfig m_display;
		TasksConfig m_tasks;
		TrayConfig m_tray;
		WorkSpacesConfig m_wspaces;
		PluginsConfig m_plugins;
		ServerConfig m_server;
		SchemeConfig m_scheme;
		WidgetsConfig m_widgets;
		DesktopWallpaperConfig m_wallpapers;
		MenuConfig m_menu;
	};
}

