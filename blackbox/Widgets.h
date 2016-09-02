#pragma once
#include <vector>
#include "WidgetsConfig.h"
#include "gfx/GuiWidget.h"
#include "Tasks.h"
#include "gfx/Gfx.h"

namespace bb {

	struct Widgets
	{
		Tasks & m_tasks;
		Gfx & m_gfx;
		std::vector<std::unique_ptr<GuiWidget>> m_widgets;
		WidgetsConfig * m_config;

		Widgets (Tasks & t, Gfx & g);
		~Widgets ();

		bool Init (WidgetsConfig & config);
		bool Done ();

		template<class T>
		T * MkWidget (WidgetConfig const & cfg)
		{
			T * t = new T;
			GfxWindow * win = m_gfx.MkGuiWindow(cfg.m_x, cfg.m_y, cfg.m_w, cfg.m_h, t->GetNameW(), t->GetNameW());
			win->m_gui->m_enabled = cfg.m_show;
			t->m_enabled = cfg.m_show;
			win->GetGui()->AddWidget(t);
			m_tasks.AddWidgetTask(win);
			return t;
		}
	};

}
