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
		T * MkWidget (WidgetConfig & cfg)
		{
			if (cfg.m_show)
			{
				T * t = new T(cfg);
				GfxWindow * win = m_gfx.MkGuiWindow(cfg.m_x, cfg.m_y, cfg.m_w, cfg.m_h, cfg.m_alpha, t->GetNameW(), t->GetNameW(), cfg.m_show);
				win->m_gui->m_enabled = cfg.m_show;
				win->GetGui()->AddWidget(t);
				t->m_hwnd = win->m_hwnd;
				m_tasks.AddWidgetTask(win);
				return t;
			}
			return nullptr;
		}
	};

}
