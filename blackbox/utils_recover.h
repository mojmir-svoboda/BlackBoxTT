#pragma once
#include <platform_win.h>

namespace bb {

	struct ProcessWindows
	{
		HWND m_hwnd = { nullptr };
		wchar_t m_appName[512] = { 0 };
		wchar_t m_caption[512] = { 0 };
		bool m_visible = { false };
	};

	size_t enumerateProcessHandles (ProcessWindows * data, size_t data_sz);
}
