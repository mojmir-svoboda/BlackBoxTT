#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct StyleEditorWidget : GuiWidget
	{
		StyleEditorWidget (WidgetConfig & cfg) : GuiWidget(cfg) { }
		virtual ~StyleEditorWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "StyleEditor"; }
		virtual wchar_t const * GetNameW () override { return L"StyleEditor"; }
	};

}

