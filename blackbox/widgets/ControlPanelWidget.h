#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct ControlPanelWidget : GuiWidget
	{
		virtual ~ControlPanelWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "ControlPanel"; }
		virtual wchar_t const * GetNameW () override { return L"ControlPanel"; }
	};

}

