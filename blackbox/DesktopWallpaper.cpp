#include "DesktopWallpaper.h"
#include <bblib/ScopeGuard.h>
#include <bblib/logging.h>
#include <BlackBox.h>
#include "Shobjidl.h"

namespace bb {

	bool DesktopWallpaper::Init ()
	{
		IDesktopWallpaper * idw = nullptr;
		if (SUCCEEDED(::CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, __uuidof(IDesktopWallpaper), (PVOID*)&idw)))
		{
			m_idw = idw;
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

}
