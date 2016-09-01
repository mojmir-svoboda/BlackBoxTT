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
	};

}
