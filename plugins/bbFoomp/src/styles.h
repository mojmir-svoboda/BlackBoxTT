#pragma once
#include <blackbox/plugin/bb.h>

struct Styles
{
	// Style items
	StyleItem OuterStyle, InnerStyle, ButtonStyle;
	COLORREF ShadowColor;

	void GetStyleSettings ();
};

Styles & getStyles ();
COLORREF GetShadowColor (StyleItem & style);
