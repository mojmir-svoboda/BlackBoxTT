#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/Tasks.h>
#include <imgui/imgui.h>

namespace bb {

	struct PagerWidget : GuiWidget
	{
		bool m_horizontal;
		ImVec2 m_contentSize { 0 , 0 };
		std::vector<TaskInfo> m_tasks;

		PagerWidget (WidgetConfig & cfg);
		virtual ~PagerWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Pager"; }
		virtual wchar_t const * GetNameW () override { return L"Pager"; }

		void UpdateTasks ();
	};

}

