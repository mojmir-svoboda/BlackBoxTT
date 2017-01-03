#pragma once
#include <memory>
#include <vector>
#include <bblib/bbstring.h>
namespace YAML { class Node; }
namespace bb {

	struct DesktopWallpaperConfig
	{
		unsigned m_position { 0 };
		unsigned m_bgColor { 0 };
		std::vector<std::vector<bbstring>> m_monitorWallpapers;

		std::vector<bbstring> m_slideShowFileNames;
		bool m_slideshowShuffle { false };
		unsigned m_slideShowTick_ms { 32000 };

		void clear () { m_slideShowFileNames.clear(); m_monitorWallpapers.clear(); }
	};

	bool loadDesktopWallpaperConfig (YAML::Node & y_root, DesktopWallpaperConfig & config);
}

