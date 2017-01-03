#include "DesktopWallpaper.h"
#include <bblib/ScopeGuard.h>
#include <bblib/logging.h>
#include <bblib/utils_paths.h>
#include <BlackBox.h>
#include "Shobjidl.h"

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
			StartSlideShow();
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

	void DesktopWallpaper::StartSlideShow ()
	{
		SetSlideShow(m_config);
	}

	void DesktopWallpaper::SetSlideShow (DesktopWallpaperConfig const & config)
	{
		std::vector<LPCITEMIDLIST> pidlChilds;

		for (bbstring const & s : config.m_slideShowFileNames)
		{
			TCHAR buffer[1024];
			TCHAR** lppPart = { nullptr };
			if (PathIsRelative(s.c_str()))
			{
				// @TODO: append curr wrk dir
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
		DESKTOP_SLIDESHOW_OPTIONS opts = config.m_slideshowShuffle ? DSO_SHUFFLEIMAGES : (DESKTOP_SLIDESHOW_OPTIONS)0;
		m_idw->SetSlideshowOptions(opts, config.m_slideShowTick_ms);
	}
}
