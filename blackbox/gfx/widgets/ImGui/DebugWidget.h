#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct DebugWidget : GuiWidget
	{
		DebugWidget (WidgetConfig & cfg) : GuiWidget(cfg) { }
		virtual ~DebugWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Debug"; }
		virtual wchar_t const * GetNameW () override { return L"Debug"; }
	};

}

