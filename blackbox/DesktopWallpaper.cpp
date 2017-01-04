#include "DesktopWallpaper.h"
#include <bblib/ScopeGuard.h>
#include <bblib/logging.h>
#include <bblib/utils_paths.h>
#include <BlackBox.h>
#include "Shobjidl.h"
#include <filesystem>

namespace bb {

	void DesktopWallpaper::GetWallpaper ()
	{
// 		UINT count = 0;
// 		m_idw->GetMonitorDevicePathCount(&count);
// 
// 		for (UINT i = 0; i < count; ++i)
// 		{
// 			LPWSTR id = nullptr;
// 			m_idw->GetMonitorDevicePathAt(i, &id);
// 
// 			RECT rc;
// 			m_idw->GetMonitorRECT(id, &rc);
// 
// 			LPWSTR wallpaper = nullptr;
// 			if (SUCCEEDED(m_idw->GetWallpaper(id, &wallpaper)))
// 			{
// 				CoTaskMemFree(wallpaper);
// 			}
// 
// 			CoTaskMemFree(id);
// 		}
	}

	bool DesktopWallpaper::Init (DesktopWallpaperConfig & config)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
		m_config = config;

		IDesktopWallpaper * idw = nullptr;
		if (SUCCEEDED(::CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, __uuidof(IDesktopWallpaper), (PVOID*)&idw)))
		{
			m_idw = idw;

			//GetWallpaper();
			ApplyConfig(m_config);
			return true;
		}
		return false;
	}

	bool DesktopWallpaper::Done ()
	{
		if (m_idw)
		{
			m_idw->Release();
			m_idw = nullptr;
		}
		return true;
	}

	void DesktopWallpaper::Enable (bool on)
	{
		if (m_idw)
		{
			BOOL b = on ? TRUE : FALSE;
			m_idw->Enable(b);
		}
	}

	// @TODO: free pidls, IShellItemArrays ?
	// @TODO: doc says to use SHParseDisplayName instead of ILCreateFromPath
	void DesktopWallpaper::ApplyConfig (DesktopWallpaperConfig const & config)
	{
		if (config.m_enabled)
		{
			std::vector<LPCITEMIDLIST> pidlChilds;

			for (bbstring const & s : config.m_slideShowFileNames)
			{
				TCHAR buffer[1024];
				TCHAR** lppPart = { nullptr };
				if (PathIsRelative(s.c_str()))
				{
					std::experimental::filesystem::path p_c = std::experimental::filesystem::current_path();
					std::experimental::filesystem::path p_s(s.c_str());

					std::experimental::filesystem::path p = p_c / p_s;

					if (std::experimental::filesystem::exists(p))
					{
						pidlChilds.push_back(ILCreateFromPath(p.c_str()));
					}
				}
				else
				{
					if (DWORD  retval = GetFullPathName(s.c_str(), 256, buffer, lppPart))
					{
						pidlChilds.push_back(ILCreateFromPath(buffer));
					}
				}
			}
		
			IShellItemArray * psiaFiles = nullptr;
			HRESULT hr = SHCreateShellItemArrayFromIDLists(pidlChilds.size(), pidlChilds.data(), &psiaFiles);

			m_idw->SetSlideshow(psiaFiles);
			DESKTOP_SLIDESHOW_OPTIONS const opts = config.m_slideshowShuffle ? DSO_SHUFFLEIMAGES : (DESKTOP_SLIDESHOW_OPTIONS)0;
			m_idw->SetSlideshowOptions(opts, config.m_slideShowTick_ms);

			DESKTOP_WALLPAPER_POSITION const pos = (DESKTOP_WALLPAPER_POSITION)config.m_position;
			m_idw->SetPosition(pos);
		}
		COLORREF col = config.m_bgColor;
		m_idw->SetBackgroundColor(col);
		Enable(config.m_enabled);
	}
}
