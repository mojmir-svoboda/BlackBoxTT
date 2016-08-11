#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/Tasks.h>

namespace bb {

	struct PagerTaskInfo : TaskInfo
	{
		PagerTaskInfo (Tasks::TaskInfoPtr const & p)
			: TaskInfo(*p.get())
		{ }
	};

	struct PagerWidget : GuiWidget
	{
		bool m_horizontal;
		std::vector<PagerTaskInfo> m_tasks;

		PagerWidget ();
		virtual ~PagerWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Pager"; }

		void UpdateTasks ();
	};

}

