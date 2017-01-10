#pragma once
#include <bblib/bbstring.h>
namespace YAML { class Node; }
namespace bb {

	struct GfxConfig
	{
		bbstring m_use;
	};

	bool loadGfxConfig (YAML::Node & y_root, GfxConfig & config);
}

