#include "Widgets.h"
#include <bblib/logging.h>
#include <widgets/StyleEditorWidget.h>
#include <widgets/PluginsWidget.h>
#include <widgets/ControlPanelWidget.h>
#include <widgets/RecoverWindowsWidget.h>
#include <widgets/TasksWidget.h>
#include <widgets/PagerWidget.h>
#include <widgets/DebugWidget.h>
#include <widgets/QuickBarWidget.h>
#include <widgets/TrayWidget.h>
#include "utils_window.h"
#include <array>

namespace bb {

Widgets::Widgets (Tasks & t, Gfx * g)
	: m_tasks(t)
	, m_gfx(g)
{
	m_widgets.reserve(16);
}

Widgets::~Widgets ()
{
	assert(m_widgets.size() == 0);
}

bool Widgets::Init (WidgetsConfig & config)
{
	TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
	m_config = &config;

	SecondMon s = { 0 };
	EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, reinterpret_cast<LPARAM>(&s));
	int const szx = 128;
	int sx = szx;
	if (s.found)
	{
		sx = s.x1;
	}

	
	for (size_t i = 0, ie = config.m_widgets.size(); i < ie; ++i)
	{
		// hey piggy...
		if (config.m_widgets[i]->m_widget == L"Pager")
			MkWidget<PagerWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"QuickBar")
			MkWidget<QuickBarWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"Tasks")
			MkWidget<TasksWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"ControlPanel")
			MkWidget<ControlPanelWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"RecoverWindows")
			MkWidget<RecoverWindowsWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"Debug")
			MkWidget<DebugWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"Plugins")
			MkWidget<PluginsWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"StyleEditor")
			MkWidget<StyleEditorWidget>(*config.m_widgets[i]);
		if (config.m_widgets[i]->m_widget == L"Tray")
			MkWidget<TrayWidget>(*config.m_widgets[i]);
	}
	return true;
}

bool Widgets::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating widgets");
	return true;
}

}
