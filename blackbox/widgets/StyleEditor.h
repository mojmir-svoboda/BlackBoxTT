#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct StyleEditorWidget : GuiWidget
	{
		virtual ~StyleEditorWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "StyleEditor"; }
	};

}

