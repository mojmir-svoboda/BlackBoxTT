#pragma once
#include <bbstring.h>

namespace bb {

	struct WidgetConfig
	{
		bbstring m_id; /// widget unique id
		bbstring m_widgetType; /// widget type
		int m_x { 0 };
		int m_y { 0 };
		int m_w { 512 };
		int m_h { 512 };
		int m_alpha { 255 - 16 };
		bool m_show { true };
		bool m_titlebar { true };
	};

}

