#pragma once
#include <memory>
#include <vector>
#include <bblib/bbstring.h>
namespace YAML { class Node; }
namespace bb {

	/*typedef enum DESKTOP_WALLPAPER_POSITION { 
		DWPOS_CENTER   = 0,
		DWPOS_TILE     = 1,
		DWPOS_STRETCH  = 2,
		DWPOS_FIT      = 3,
		DWPOS_FILL     = 4,
		DWPOS_SPAN     = 5
	} DESKTOP_WALLPAPER_POSITION;*/

	struct DesktopWallpaperConfig
	{
		bool m_enabled { true };
		unsigned m_position { 0 }; // DESKTOP_WALLPAPER_POSITION
		unsigned m_bgColor { 0 };
		std::vector<std::vector<bbstring>> m_monitorWallpapers;

		std::vector<bbstring> m_slideShowFileNames;
		bool m_slideshowShuffle { false };
		unsigned m_slideShowTick_ms { 32000 };

		void clear () { m_slideShowFileNames.clear(); m_monitorWallpapers.clear(); }
	};

	bool loadDesktopWallpaperConfig (YAML::Node & y_root, DesktopWallpaperConfig & config);
}

