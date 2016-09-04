#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/Tasks.h>

namespace bb {

	struct PagerWidget : GuiWidget
	{
		bool m_horizontal;
		std::vector<TaskInfo> m_tasks;

		PagerWidget (WidgetConfig & cfg);
		virtual ~PagerWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Pager"; }
		virtual wchar_t const * GetNameW () override { return L"Pager"; }

		void UpdateTasks ();
	};

}

