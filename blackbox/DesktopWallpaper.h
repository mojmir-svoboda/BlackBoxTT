#pragma once
#include <bblib/bbstring.h>
#include <platform_win.h>
struct IDesktopWallpaper;

namespace bb {

	struct DesktopWallpaper
	{
		IDesktopWallpaper * m_idw { nullptr };
		bool Init ();
		bool Done ();
	};

}
