#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct DebugWidget : GuiWidget
	{
		virtual ~DebugWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Debug"; }
	};

}

