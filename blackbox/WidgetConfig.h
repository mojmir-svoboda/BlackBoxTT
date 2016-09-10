#pragma once
#include <bbstring.h>

namespace bb {

	struct WidgetConfig
	{
		bbstring m_widget;
		int m_x { 0 };
		int m_y { 0 };
		int m_w { 512 };
		int m_h { 512 };
		int m_alpha { 255 - 16 };
		bool m_show { false };
		bool m_vertical { false };
		bool m_titlebar { true };
	};

}

