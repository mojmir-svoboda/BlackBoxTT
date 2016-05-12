#pragma once
#include <imgui/imgui.h>
#include <platform_win.h>
#include <vector>

namespace bb
{
	struct GuiWidget
	{
		bool m_enabled;

		GuiWidget () : m_enabled(false) { }
		virtual ~GuiWidget () { }
		virtual void DrawUI () { }
		virtual char const * GetName () = 0;
	};
}

