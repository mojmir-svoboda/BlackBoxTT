#include "styles.h"
#include "settings.h"
#include <bblibcompat/styleprops.h>
#include <bblibcompat/StyleStruct.h>

Styles g_styles;
Styles & getStyles () { return g_styles; } // "singleton"

COLORREF MakeShadowColor(StyleItem &style)
{
	int rav, gav, bav;
	if (style.type != B_SOLID)
	{
		rav = (GetRValue(style.Color)+GetRValue(style.ColorTo)) / 2;
		gav = (GetGValue(style.Color)+GetGValue(style.ColorTo)) / 2;
		bav = (GetBValue(style.Color)+GetBValue(style.ColorTo)) / 2;
	}
	else
	{
		rav = GetRValue(style.Color);
		gav = GetGValue(style.Color);
		bav = GetBValue(style.Color);
	}

	if (rav < 0x30) rav = 0;
	else rav -= 0x10;
	if (gav < 0x30) gav = 0;
	else gav -= 0x10;
	if (bav < 0x30) bav = 0;
	else bav -= 0x10;

	return RGB((BYTE)rav, (BYTE)gav, (BYTE)bav);
}

COLORREF GetShadowColor (StyleItem & style)
{
	return (style.validated & VALID_SHADOWCOLOR) ?	style.ShadowColor : MakeShadowColor(style);
}

void Styles::GetStyleSettings()
{
	int const style_assoc[] =
	{ 
		SN_TOOLBARLABEL,
		SN_TOOLBARWINDOWLABEL,
		SN_TOOLBARCLOCK,
		SN_TOOLBAR,
		SN_TOOLBARBUTTON,
		SN_TOOLBARBUTTONP
	};

	StyleItem const * os = (StyleItem *)GetSettingPtr(style_assoc[getSettings().OuterStyleIndex-1]);
	StyleItem const * is = (StyleItem *)GetSettingPtr(style_assoc[getSettings().InnerStyleIndex-1]);
	StyleItem const * toolbar = (StyleItem *)GetSettingPtr(SN_TOOLBAR);
	StyleItem const * toolbarButtonPressed = (StyleItem *)GetSettingPtr(SN_TOOLBARBUTTONP);

	OuterStyle = os->parentRelative ? *toolbar : *os;
	InnerStyle = is->parentRelative ? *toolbar : *is;
	ButtonStyle = *toolbarButtonPressed;

//	SIZE size;
	HDC fonthdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	HFONT font = CreateStyleFont(toolbar);
	HGDIOBJ oldfont = SelectObject(fonthdc, font);
//	GetTextExtentPoint32(fonthdc, foobarWnd, 32, &size);
	getSettings().width = getSettings().FooWidth;
	DeleteObject(SelectObject(fonthdc, oldfont));
	DeleteDC(fonthdc);
}


