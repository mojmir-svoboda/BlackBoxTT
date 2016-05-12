#pragma once
#include "plugin.h"

namespace bb {

	struct NativePluginInfo
	{
		HMODULE m_module;

		beginPlugin_t m_beginPlugin;
		beginPluginEx_t m_beginPluginEx;
		beginSlitPlugin_t m_beginSlitPlugin;
		endPlugin_t m_endPlugin;
		pluginInfo_t m_pluginInfo;
	};
}

