#pragma once
#include <vector>
#include <bblib/bbstring.h>
namespace YAML { class Node; }

namespace bb {

	struct WorkSpaceConfig
	{
    //bb_hash_t m_id; /// hash from name
		bbstring m_name;
	};

	struct WorkGroupConfig
	{
    //bb_hash_t m_id; /// hash from name
		bbstring m_name;
		bbstring m_hotkey;
		uint16_t m_x;
		uint16_t m_y;
    bool m_circularX;
    bool m_circularY;
		bbstring m_current;
    std::vector<WorkSpaceConfig> m_wspaces;
	};

	struct WorkSpacesConfig
	{
		bbstring m_current;
    std::vector<WorkGroupConfig> m_groups;
	};

	bool loadWorkSpacesConfig (YAML::Node & y_root, WorkSpacesConfig & config);
}

