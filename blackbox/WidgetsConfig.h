#pragma once
#include "WidgetConfig.h"
#include <memory>
#include <vector>
namespace YAML { class Node; }
namespace bb {

	struct WidgetsConfig
	{
		std::vector<std::unique_ptr<WidgetConfig>> m_widgets;

		void clear () { m_widgets.clear(); }
	};

	bool loadWidgetsConfig (YAML::Node & y_root, WidgetsConfig & config);
}

