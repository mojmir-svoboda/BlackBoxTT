#pragma once
#include <bbstring.h>

namespace bb {

	struct WidgetConfig
	{
		bbstring m_widget;
		unsigned m_x { 0 };
		unsigned m_y { 0 };
		unsigned m_w { 0 };
		unsigned m_h { 0 };
		bool m_show { false };
		bool m_vertical { false };
	};

}

