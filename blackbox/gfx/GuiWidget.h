#pragma once
#include <imgui/imgui.h>
#include <platform_win.h>
#include <vector>
#include <WidgetConfig.h>
#include "GfxWindow.h"

namespace bb
{
	struct GuiWidget
	{
		WidgetConfig & m_config;
		HWND m_hwnd { nullptr };
		GfxWindow * m_gfxWindow { nullptr };

		GuiWidget (WidgetConfig & cfg) : m_config(cfg) { }
		virtual ~GuiWidget () { }
		virtual void DrawUI () { }
		virtual char const * GetName () = 0;
		virtual wchar_t const * GetNameW () = 0;
		void Show (bool on) { m_gfxWindow->Show(on); }
		bool Enabled () const { return m_gfxWindow->Enabled(); }
	};
}

