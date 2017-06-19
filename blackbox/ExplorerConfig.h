#pragma once

namespace YAML { class Node; }
namespace bb {

	struct ExplorerConfig
	{
		bool m_show { false };
	};

	bool loadExplorerConfig (YAML::Node & y_root, ExplorerConfig & config);
}

