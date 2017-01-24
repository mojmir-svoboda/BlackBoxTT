#pragma once
#include <bblib/bbstring.h>
#include <vector>
namespace YAML { class Node; }
namespace bb {

	struct GfxConfig
	{
		bbstring m_use;
		std::vector<bbstring> m_startWidgets;

		void clear () { m_startWidgets.clear(); }
	};

	bool loadGfxConfig (YAML::Node & y_root, GfxConfig & config);
}

