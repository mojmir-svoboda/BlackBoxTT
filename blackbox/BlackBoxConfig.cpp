#include "BlackBoxConfig.h"
#include <platform_win.h>
#include <bblib/logging.h>
#include <VersionHelpers.h>

namespace bb {

	bool OSConfig::Init ()
	{
		m_usingXP = ::IsWindowsXPOrGreater();
		m_usingVista = ::IsWindowsVistaOrGreater();
		m_usingWin7 = ::IsWindows7OrGreater();
		m_usingWin8 = ::IsWindows8OrGreater();
		m_usingWin10 = false;
		//cfg.m_usingWin10 = IsWindows10OrGreater();
		return true;
	}

	bool DisplayConfig::Init ()
	{
		m_monitors = GetSystemMetrics(SM_CMONITORS);
		if (m_monitors > 1)
		{
			m_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
			m_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
			m_w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
			m_h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		}
		else
		{
			m_x = 0;
			m_y = 0;
			m_w = GetSystemMetrics(SM_CXSCREEN);
			m_h = GetSystemMetrics(SM_CYSCREEN);
		}
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT, "Monitor count: %d", m_monitors);
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT, "workspace resolution: %d x %d", m_w, m_h);
		return m_monitors > 0;
	}

	bool DisplayConfig::Update ()
	{
		DisplayConfig rhs;
		rhs.Init();
		if (*this != rhs)
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "DisplayConfig: change detected");
			*this = rhs;
			return true;
		}
		return false;
	}
}

