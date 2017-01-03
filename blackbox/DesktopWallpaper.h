#pragma once
#include <bblib/bbstring.h>
#include <platform_win.h>
#include "DesktopWallpaperConfig.h"
struct IDesktopWallpaper;

namespace bb {

	struct DesktopWallpaper
	{
		DesktopWallpaperConfig m_config;
		IDesktopWallpaper * m_idw { nullptr };

		bool Init (DesktopWallpaperConfig &);
		bool Done ();

		void GetWallpaper ();
		void SetSlideShow (DesktopWallpaperConfig const &);
		void StartSlideShow ();
	};

}
