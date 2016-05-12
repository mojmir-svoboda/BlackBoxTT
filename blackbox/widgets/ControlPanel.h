#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct ControlPanelWidget : GuiWidget
	{
		virtual ~ControlPanelWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Ctrl"; }
	};

}

