#pragma once
#include <blackbox/gfx/GuiWidget.h>

namespace bb {

	struct TasksWidget : GuiWidget
	{
		bool m_horizontal;

		virtual ~TasksWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Tasks"; }
	};

}

