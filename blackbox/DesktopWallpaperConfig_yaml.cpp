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

			YAML::Node node = y_root["Wallpapers"]; // @TODO: unicode? utf8?

			if (node["slideShowFiles"])
				rhs.m_slideShowFileNames = node["slideShowFiles"].as<std::vector<bbstring>>();

			if (node["slideShowShuffle"])
				rhs.m_slideshowShuffle = node["slideShowShuffle"].as<bool>();

			if (node["slideShowTick"])
				rhs.m_slideShowTick_ms = node["slideShowTick"].as<unsigned>();

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
