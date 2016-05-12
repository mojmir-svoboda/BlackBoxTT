#pragma once

namespace bb {

	struct WorkSpaceConfig
	{
		uint16_t m_x;
		uint16_t m_y;
	};

	struct GroupConfig
	{
		bbstring m_name;
		WorkSpaceConfig m_wspace;
	};

	struct WorkSpacesConfig
	{
		GroupConfig m_group;
	};
}

