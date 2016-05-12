#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include "TaskConfig.h"
#include "gfx/IconId.h"

namespace bb {

	struct TaskInfo
	{
		TaskConfig * m_config;
		HWND		m_hwnd;
		HICON		m_icon;
		IconId	m_icoSmall;
		IconId	m_icoLarge;
		int			m_wkspc;
		bbstring m_caption;
		bbstring m_tag;

		bool		m_active;
		bool		m_flashing;
		bool		m_fullscreen;

		bool		m_sticky; /// window is sticky
		bool		m_ignored; /// window is present on screen, but not in task bar

		TaskInfo ()
			: m_config(nullptr), m_hwnd(), m_icon(nullptr), m_wkspc(0), m_active(false), m_flashing(false), m_fullscreen(false)
		{ }
		TaskInfo (HWND hwnd)
			: m_config(nullptr), m_hwnd(hwnd), m_icon(nullptr), m_wkspc(0), m_active(false), m_flashing(false), m_fullscreen(false)
		{ }
	};

}
