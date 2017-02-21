#include "MenuWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/utils_window.h>
#include <bblib/codecvt.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <blackbox/menu/MenuConfig_yaml.h>

namespace bb {

	bool MenuWidget::loadConfig (YAML::Node & y_cfg_node)
	{
		if (!y_cfg_node.IsNull())
		{
			MenuConfig tmp = y_cfg_node.as<MenuConfig>();
			m_config = std::move(tmp);
			return true;
		}
		return false;
	}

}
