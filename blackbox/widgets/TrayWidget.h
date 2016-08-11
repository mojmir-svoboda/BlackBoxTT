#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct TrayWidget : GuiWidget
	{
		virtual ~TrayWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Ctrl"; }
	};

}

