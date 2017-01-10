#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/Tasks.h>

namespace bb {

	struct RecoverWindowsWidget : GuiWidget
	{
		std::vector<TaskInfo> m_tasks;
		std::vector<size_t> m_order;
		std::vector<bbstring> m_exenames;

		RecoverWindowsWidget (WidgetConfig & cfg);
		virtual ~RecoverWindowsWidget () { }
		virtual void DrawUI () override;
		virtual char const * GetName () override { return "RecoverWindows"; }
		virtual wchar_t const * GetNameW () override { return L"RecoverWindows"; }

		void UpdateData ();
	};

}

