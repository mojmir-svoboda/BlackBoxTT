#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct QuickBarWidget : GuiWidget
	{
		bool m_horizontal;

		QuickBarWidget (WidgetConfig & cfg);
		virtual ~QuickBarWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "QuickBar"; }
		virtual wchar_t const * GetNameW () override { return L"QuickBar"; }
	};

}

