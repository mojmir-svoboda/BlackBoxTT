#pragma once
#include <platform_win.h>
#include <vector>
#include <WidgetConfig.h>
#include "GfxWindow.h"

namespace bb
{
	struct GuiWidget
	{
		bool m_show { false };
		GfxWindow * m_gfxWindow { nullptr };
		WidgetConfig m_config;

		GuiWidget (WidgetConfig & cfg) : m_config(cfg) { }
		virtual ~GuiWidget () { }
		virtual void DrawUI () { }
		virtual char const * GetName () = 0;
		virtual wchar_t const * GetNameW () = 0;

		virtual void Show (bool on) { m_show = on; }
		virtual bool Visible () const { return m_show; }
	};
}

