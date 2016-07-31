#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct PagerWidget : GuiWidget
	{
		bool m_horizontal;

		virtual ~PagerWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Pager"; }
	};

}

