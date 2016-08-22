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
		HICON		m_icon        { nullptr };
		IconId	m_icoSmall    { };
		IconId	m_icoLarge    { };

		enum : size_t { e_wspaceLenMax = 64 };
		wchar_t m_wspace[e_wspaceLenMax] = { 0 };
		enum : size_t { e_captionLenMax = 512 };
		wchar_t m_caption[e_captionLenMax] = { 0 };

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
	};

}
