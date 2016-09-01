#include "Widgets.h"
#include <bblib/logging.h>
#include <widgets/StyleEditorWidget.h>
#include <widgets/PluginsWidget.h>
#include <widgets/ControlPanelWidget.h>
#include <widgets/RecoverWindowsWidget.h>
#include <widgets/TasksWidget.h>
#include <widgets/PagerWidget.h>
#include <widgets/DebugWidget.h>
#include "utils_window.h"

namespace bb {

Widgets::Widgets (Tasks & t, Gfx & g)
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
//		GfxWindow * w0 = m_gfx.MkGuiWindow(sx - szx, s.y0, szx, szx, L"c0", L"w0");
//		{
//			w0->m_gui->m_enabled = true;
//			DebugWidget * w0wdg0 = new DebugWidget;
//			w0wdg0->m_enabled = true;
//			w0->GetGui()->AddWidget(w0wdg0);
//		}
	GfxWindow * w0 = m_gfx.MkGuiWindow(0, 0, 200, 200, L"bbPager", L"bbPager");
	{
		w0->m_gui->m_enabled = true;
		PagerWidget * w0wdg0 = new PagerWidget;
		w0wdg0->m_enabled = true;
		w0->GetGui()->AddWidget(w0wdg0);
		m_tasks.AddWidgetTask(w0);
	}

	{
		GfxWindow * w1 = m_gfx.MkGuiWindow(0, 200, 200, 600, L"bbTasks", L"bbTasks");
		w1->m_gui->m_enabled = true;
		TasksWidget * w1wdg0 = new TasksWidget;
		w1wdg0->m_enabled = true;
		w1wdg0->m_horizontal = false;
		w1->GetGui()->AddWidget(w1wdg0);
	}

	{
		GfxWindow * w2 = m_gfx.MkGuiWindow(0, 800, 400, 600, L"bbRecoverWindows", L"bbRecoverWindows");
		w2->m_gui->m_enabled = true;
		RecoverWindowsWidget * w2wdg0 = new RecoverWindowsWidget;
		w2wdg0->m_enabled = true;
		w2->GetGui()->AddWidget(w2wdg0);
	}


//		GfxWindow * w1 = m_gfx.MkGuiWindow(0, 200, 800, 600, L"bbStyleEditor", L"bbStyleEditor");
//		{
//			w1->m_gui->m_enabled = true;
//			StyleEditorWidget * w1wdg0 = new StyleEditorWidget;
//			w1wdg0->m_enabled = true;
//			w1->GetGui()->AddWidget(w1wdg0);
//		}


	return true;
}

bool Widgets::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating widgets");
	return true;
}

}
