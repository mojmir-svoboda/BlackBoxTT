#include "Widgets.h"
#include <bblib/logging.h>
#include <gfx/widgets/ImGui/StyleEditorWidget.h>
#include <gfx/widgets/ImGui/PluginsWidget.h>
#include <gfx/widgets/ImGui/ControlPanelWidget.h>
#include <gfx/widgets/ImGui/RecoverWindowsWidget.h>
#include <gfx/widgets/ImGui/TasksWidget.h>
#include <gfx/widgets/ImGui/PagerWidget.h>
#include <gfx/widgets/ImGui/DebugWidget.h>
#include <gfx/widgets/ImGui/QuickBarWidget.h>
#include <gfx/widgets/ImGui/TrayWidget.h>
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
