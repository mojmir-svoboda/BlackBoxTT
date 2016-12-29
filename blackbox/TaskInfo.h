#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include "TaskConfig.h"
#include "gfx/IconId.h"

namespace bb {

	struct TaskInfo
	{
		TaskConfig * m_config { nullptr };
		HWND		m_hwnd        { nullptr };
		bool		m_uwp					{ false };
		IconId	m_icoSmall    { };
		IconId	m_icoLarge    { };

		enum : size_t { e_wspaceLenMax = 64 };
		wchar_t m_wspace[e_wspaceLenMax] = { 0 };
		enum : size_t { e_captionLenMax = 512 };
		wchar_t m_caption[e_captionLenMax] = { 0 };
		enum : size_t { e_appNameLenMax = 512 };
		wchar_t m_appName[e_appNameLenMax] = { 0 };

		TaskInfo (HWND hwnd)
			: m_hwnd(hwnd)
		{ }

		void SetWorkSpace (wchar_t const * s)
		{
			wcsncpy(m_wspace, s, e_wspaceLenMax);
		}

		void SetCaption (wchar_t const * s)
		{
			wcsncpy(m_caption, s, e_captionLenMax);
		}

		bool IsTaskManIgnored () const
		{
			if (m_config)
				return m_config->m_taskman == false;
			return false;
		}

		bool IsSticky () const
		{
			if (m_config)
				return m_config->m_sticky;
			return false;
		}
	};

}
