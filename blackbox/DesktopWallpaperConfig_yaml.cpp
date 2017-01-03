#include "DesktopWallpaperConfig.h"
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"

namespace bb {

	bool loadDesktopWallpaperConfig (YAML::Node & y_root, DesktopWallpaperConfig & rhs)
	{
		try
		{
			if (y_root.IsNull())
				return false;

			YAML::Node node = y_root["Wallpapers"];

			if (node["enabled"])
				rhs.m_enabled = node["enabled"].as<bool>();

			if (node["slideShowFiles"])
				rhs.m_slideShowFileNames = node["slideShowFiles"].as<std::vector<bbstring>>();

			if (node["slideShowShuffle"])
				rhs.m_slideshowShuffle = node["slideShowShuffle"].as<bool>();

			if (node["slideShowTick"])
				rhs.m_slideShowTick_ms = node["slideShowTick"].as<unsigned>();

			if (node["position"])
				rhs.m_position = node["position"].as<unsigned>();

			if (node["bgColor"])
				rhs.m_bgColor = node["bgColor"].as<unsigned>();

			return true;
		}
		catch (std::exception & e)
		{
			TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML exception in source %s: %s", __FILE__, e.what());
			return false;
		}
		return false;
	}

}
