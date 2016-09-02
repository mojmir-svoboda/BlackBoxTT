#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/Tasks.h>

namespace bb {

	struct TasksWidget : GuiWidget
	{
		std::vector<TaskInfo> m_tasks;

		bool m_horizontal;

		TasksWidget ();
		virtual ~TasksWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Tasks"; }
		virtual wchar_t const * GetNameW () override { return L"Tasks"; }

		void UpdateTasks ();
	};

}

