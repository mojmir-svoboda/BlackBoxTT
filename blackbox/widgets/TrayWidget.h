#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct TrayWidget : GuiWidget
	{
		TrayWidget (WidgetConfig & cfg) : GuiWidget(cfg) { }
		virtual ~TrayWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Tray"; }
		virtual wchar_t const * GetNameW () override { return L"Tray"; }
	};

}

