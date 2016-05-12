#pragma once
namespace YAML { class Node; }
namespace bb {

	struct TrayConfig
	{
		bool m_on;
	};

	bool loadTrayConfig (YAML::Node & y_root, TrayConfig & config);
}

