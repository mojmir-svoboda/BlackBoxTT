#include "Tray.h"
#include "platform_win.h"
#include <hooks/trayhook.h>
#include <bblib/logging.h>

namespace bb {

	bool Tray::Init (TrayConfig & cfg)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "Initializing tray");
		return true;
	}

	bool Tray::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB, "Terminating tray");
		return true;
	}

	void Tray::EnumTray ()
	{
	}

	void Tray::UpdateFromTrayHook ()
	{
		TrayData const * hook_data = g_trayData;
		m_data.m_lock.Lock();
		hook_data->m_lock.Lock();

		size_t const n = hook_data->m_count;
		for (size_t i = 0; i < n; ++i)
		{
			m_data.m_data[i] = hook_data->m_data[i];
		}
		m_data.m_count = n;

		hook_data->m_lock.Unlock();
		m_data.m_lock.Unlock();
	}

}
