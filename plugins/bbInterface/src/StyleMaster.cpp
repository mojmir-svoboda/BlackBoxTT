#include <blackbox/plugin/bb.h>
#include <blackbox/BlackBox_compat.h>
#include <bblibcompat/StyleStruct.h>
#ifdef _MSC_VER
# pragma comment(lib, "msimg32.lib")
#endif

#include "StyleMaster.h"
#include "PluginMaster.h"
#include "ControlMaster.h"

const wchar_t * szStyleNames[STYLE_COUNT + 1] = {L"Toolbar", L"Inset", L"Flat", L"Sunken", L"Label", L"Clock", L"Button", L"PressedButton", L"None", NULL};
// do a full changeable style property listing, with the property types in a separate list
const wchar_t * szStyleProperties[STYLE_PROPERTY_COUNT + 1] = 
{
		L"Color",
		L"ColorTo",
		L"TextColor",
		L"Bevel",
		L"BevelPosition",
		L"BorderWidth",
		L"BorderColor",
		L"ShadowColor",
		L"ShadowPosX",
		L"ShadowPosY",
		NULL
};
const wchar_t * szBevelTypes[STYLE_BEVEL_TYPE_COUNT + 1] = {L"Flat", L"Raised", L"Sunken", NULL};

// use only button
const wchar_t * szPressedStyleNames[PRESSED_STYLE_COUNT + 1] = {L"Toolbar", L"Inset", L"Flat", L"Sunken", L"Label", L"Clock", L"Button", L"PressedButton" , L"None", L"Default", NULL};
const wchar_t * szPressedStyleProperties[STYLE_PROPERTY_COUNT + 1] = 
{
		L"PressedColor", 
		L"PressedColorTo", 
		L"PressedTextColor", 
		L"PressedBevel", 
		L"PressedBevelPosition",
		L"PressedBorderWidth",
		L"PressedBorderColor",
		L"PressedShadowColor", 
		L"PressedShadowPosX",
		L"PressedShadowPosY",
		NULL
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BOOL trans_BitBlt(
	HDC hdcDst, int xd, int yd, int width, int height,
	HDC hdcSrc, int xs, int ys,
	int mode
	);

#define BMPTRANS 0
#define TGATRANS 1

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA 1
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//Colors
/* //These seem unused to me. Text search says so too.
COLORREF    style_color_font;
COLORREF    style_color[STYLE_COUNT];
COLORREF    style_color_to[STYLE_COUNT];
*/
COLORREF    style_color_border;


#define TRANSCOLOUR  RGB(255,0,255)
#define VALID_TEXTCOLOR 8

//Fonts
HFONT       style_font;

//Style info
StyleItem   style_fill[STYLE_COUNT];

int         style_bevel_width;
int         style_border_width;
int         style_font_height;

BOOL (WINAPI *pSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int get_font_height(HFONT hFont)
{
	TEXTMETRIC TXM;
	HDC hdc = CreateCompatibleDC(NULL);
	HGDIOBJ other = SelectObject(hdc, hFont);
	GetTextMetrics(hdc, &TXM);
	SelectObject(hdc, other);
	DeleteDC(hdc);
	return TXM.tmHeight;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_init
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int style_startup()
{
	// --------------------
	StyleItem * T = (StyleItem *)GetSettingPtr(SN_TOOLBAR);
	StyleItem * W = (StyleItem *)GetSettingPtr(SN_TOOLBARWINDOWLABEL);
	StyleItem * L = (StyleItem *)GetSettingPtr(SN_TOOLBARLABEL);
	StyleItem * C = (StyleItem *)GetSettingPtr(SN_TOOLBARCLOCK);
	StyleItem * B = (StyleItem *)GetSettingPtr(SN_TOOLBARBUTTON);
	StyleItem * P = (StyleItem *)GetSettingPtr(SN_TOOLBARBUTTONP);
	StyleItem * X;
// 	if (BBP_is_bbversion_lean())
// 	{
		T->ShadowX = T->ShadowY = 1;
		W->ShadowX = W->ShadowY = 1;
		L->ShadowX = L->ShadowY = 1;
		C->ShadowX = C->ShadowY = 1;
		B->ShadowX = B->ShadowY = 1;
//	}
	if (T->validated & VALID_TEXTCOLOR) X = T;
	else
	if (W->parentRelative)  X = W;
	else
	if (L->parentRelative)  X = L;
	else
	if (C->parentRelative)  X = C;
	else
	if (B->parentRelative)  X = B;
	else                    X = L;

	COLORREF Color_N = X->TextColor;

	if (false == W->parentRelative) X = W;
	else
	if (false == L->parentRelative) X = L;
	else
	if (false == C->parentRelative) X = C;
	else
	if (false == B->parentRelative) X = B;
	else                            X = T;

	StyleItem * A = X;

	if (X == T) X = W;

	COLORREF Color_A = X->TextColor;

	style_fill[STYLETYPE_TOOLBAR] = *T;
	style_fill[STYLETYPE_TOOLBAR].TextColor = Color_N;
	style_fill[STYLETYPE_INSET] = *A;
	style_fill[STYLETYPE_INSET].TextColor = Color_A;

	style_fill[STYLETYPE_FLAT] = style_fill[STYLETYPE_TOOLBAR];
	style_fill[STYLETYPE_FLAT].ColorTo = style_fill[STYLETYPE_TOOLBAR].Color;
	style_fill[STYLETYPE_SUNKEN] = style_fill[STYLETYPE_INSET];
	style_fill[STYLETYPE_SUNKEN].ColorTo = style_fill[STYLETYPE_INSET].Color;

	style_fill[STYLETYPE_LABEL] = (L->parentRelative)?*T:*L;
	style_fill[STYLETYPE_CLOCK] = (C->parentRelative)?*T:*C;
	style_fill[STYLETYPE_BUTTON] = (B->parentRelative)?*T:*B;
	style_fill[STYLETYPE_PRESSEDBUTTON] = (P->parentRelative)?*T:*P;
	
	style_fill[STYLETYPE_NONE].TextColor = Color_N;

 
        style_font = CreateStyleFont(T);
	        
	*(FARPROC*)&pSetLayeredWindowAttributes
		= GetProcAddress(GetModuleHandle(L"USER32"), "SetLayeredWindowAttributes");
	// --------------------

	style_bevel_width   = *(int*)GetSettingPtr(SN_BEVELWIDTH);
	style_border_width  = *(int*)GetSettingPtr(SN_BORDERWIDTH);
	style_color_border  = *(COLORREF*)GetSettingPtr(SN_BORDERCOLOR);

	style_font_height   = get_font_height(style_font);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int style_shutdown()
{
	if (style_font)
		DeleteObject(style_font), style_font = NULL;

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_set_transparency
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_set_transparency(HWND hwnd, BYTE transvalue, bool transback)
{
	if (pSetLayeredWindowAttributes)
	{
		LONG_PTR s = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		UINT flags = 0;
		if (transback) flags |= LWA_COLORKEY;
		if (transvalue < 255) flags |= LWA_ALPHA;
		if (flags)
		{
			if (0 == (s & WS_EX_LAYERED))
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, s | WS_EX_LAYERED);
			pSetLayeredWindowAttributes(hwnd, transback ? TRANSCOLOUR : 0, transvalue, flags);
		}
		else
		{
			if (0 != (s & WS_EX_LAYERED))
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, s & ~WS_EX_LAYERED);
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_check_transparency_workaround
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_check_transparency_workaround(HWND hwnd)
{
	HWND pw = GetParent(hwnd);
	if (pw && (WS_EX_LAYERED & GetWindowLongPtr(pw, GWL_EXSTYLE)))
		InvalidateRect(hwnd, NULL, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_delete_bitmaps
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_invalidate(control *c)
{
	window *w = c->windowptr;
	if (w->bitmap) DeleteObject(w->bitmap), w->bitmap = NULL;
	InvalidateRect(w->hwnd, NULL, FALSE);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_begin
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool style_draw_begin(control *c, styledrawinfo &d)
{
	window *w = c->windowptr;
	d.hwnd = w->hwnd;

	//get the screen-buffer
	d.devicecontext = BeginPaint(d.hwnd, &d.ps);

	d.rect.left = d.rect.top = 0;
	d.rect.right = w->width;
	d.rect.bottom = w->height;
	d.apply_sat_hue = true;

	bool must_paint = NULL == w->bitmap;
	// no need to paint all the gradient and bitmap stuff,
	// if nothing didn't change. This is fast!
	if (must_paint)
	{
		//then create a new buffer in memory with the window's size
		w->bitmap = CreateCompatibleBitmap(d.devicecontext, d.rect.right, d.rect.bottom);
	}

	//first get a new 'device context'
	d.buffer = CreateCompatibleDC(d.devicecontext);

	//select it into the DC and store, whatever there was before.
	d.otherbmp = SelectObject(d.buffer, w->bitmap);

	return must_paint;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_end
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_end(styledrawinfo &d)
{
	//now copy the drawing from the paint buffer to the window...
	BitBlt(
		d.devicecontext,
		d.ps.rcPaint.left,
		d.ps.rcPaint.top,
		d.ps.rcPaint.right  - d.ps.rcPaint.left,
		d.ps.rcPaint.bottom - d.ps.rcPaint.top,
		d.buffer,
		d.ps.rcPaint.left,
		d.ps.rcPaint.top,
		SRCCOPY
		);

	//restore the previous GDI-object to the dc
	SelectObject(d.buffer, d.otherbmp);

	//delete the memory - 'device context'
	DeleteDC(d.buffer);

	//done
	EndPaint(d.hwnd, &d.ps);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_box
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_box(RECT &rect, HDC &buffer, int style, bool is_bordered)
{
	if (style == STYLETYPE_NONE)
	{
		HBRUSH hbbrush = CreateSolidBrush(TRANSCOLOUR);
		FillRect(buffer, &rect, hbbrush);
		DeleteObject(hbbrush);
		return;
	}

	if (style >= STYLE_COUNT) return;
	if (rect.right - rect.left < 2) return;
	if (rect.bottom - rect.top < 2) return;

	//Draw the box appropriately
	MakeStyleGradient(buffer, &rect, &style_fill[style], is_bordered);
}

COLORREF style_make_shadowcolor(StyleItem* style)
{
	int rav, gav, bav;
	if (style->type != B_SOLID)
	{
		rav = (GetRValue(style->Color)+GetRValue(style->ColorTo)) / 2;
		gav = (GetGValue(style->Color)+GetGValue(style->ColorTo)) / 2;
		bav = (GetBValue(style->Color)+GetBValue(style->ColorTo)) / 2;
	}
	else
	{
		rav = GetRValue(style->Color);
		gav = GetGValue(style->Color);
		bav = GetBValue(style->Color);
	}

	if (rav < 0x20) rav = 0;
	else rav -= 0x20;
	if (gav < 0x20) gav = 0;
	else gav -= 0x20;
	if (bav < 0x20) bav = 0;
	else bav -= 0x20;

	return RGB((BYTE)rav, (BYTE)gav, (BYTE)bav);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_box
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_box(RECT &rect, HDC &buffer, StyleItem* styleptr, bool is_bordered)
{
	if (!styleptr) return;

	if (rect.right - rect.left < 2) return;
	if (rect.bottom - rect.top < 2) return;

	//Draw the box appropriately
	MakeStyleGradient(buffer, &rect, styleptr, is_bordered);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_text
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_text(RECT &rect, HDC &buffer, StyleItem* styleptr, char *text, UINT settings, HFONT font)
{
	HFONT *newfont = font!=NULL?&font:&style_font;
	
	//Set the font object, and get the last object
	HGDIOBJ previous = SelectObject(buffer, *newfont);

	//Setup drawing
	SetBkMode(buffer, TRANSPARENT);

	//Parse it for newline characters
	wchar_t temp[BBI_MAX_LINE_LENGTH], *p = temp, c;
	do
	{
		c = *p = *text++;
		if (c == L'\\' && *text == L'n')
			*p++ = L'\r', *p = L'\n', text++;
		p++;
	} while (c);

	//Draw shadow
	if(styleptr->validated & VALID_SHADOWCOLOR && styleptr->ShadowXY != 0){
		RECT s;
		s.left = rect.left + styleptr->ShadowX;
		s.top = rect.top + styleptr->ShadowY;
		s.bottom = rect.bottom + styleptr->ShadowY;
		s.right = rect.right + styleptr->ShadowX;
		if(styleptr->ShadowColor != CLR_INVALID){
			SetTextColor(buffer, styleptr->ShadowColor);
		}else{
			SetTextColor(buffer,style_make_shadowcolor(styleptr));
		}
		DrawText(buffer, temp, -1, &s, DT_NOPREFIX | settings);
	}

	//Draw the text
	SetTextColor(buffer, styleptr->TextColor);
	DrawText(buffer, temp, -1, &rect, DT_NOPREFIX | settings);

	//Set the last object again
	SelectObject(buffer, previous);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_text
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_text(RECT &rect, HDC &buffer, int style, char *text, UINT settings, HFONT font)
{
	style_draw_text(rect, buffer, &style_fill[style], text, settings, font);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//style_draw_image_alpha
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void style_draw_image_alpha(HDC &buffer, int x, int y, int width, int height, HANDLE &image)
{
	HDC hdcMem = CreateCompatibleDC(buffer);
	HBITMAP old = (HBITMAP) SelectObject(hdcMem, image);

	const BLENDFUNCTION pixelblend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

	//if (plugin_using_modern_os) AlphaBlend(buffer, x, y, width, height, hdcMem, 0, 0, width, height, RGB(255,0,255));
	if (plugin_using_modern_os) AlphaBlend(buffer, x, y, width, height, hdcMem, 0, 0, width, height, pixelblend);
	else trans_BitBlt(buffer, x, y, width, height, hdcMem, 0, 0, TGATRANS);

	SelectObject(hdcMem, old);
	DeleteDC(hdcMem);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// style_get_copy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
StyleItem style_get_copy(int styleidx)
{
	if (styleidx < STYLE_COUNT) return style_fill[styleidx];
	return style_fill[0]; //Somewhat shabby. Assuming that there is at least one style, this works.
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// style_get_copy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
COLORREF style_get_text_color(int styleidx)
{
	if (styleidx < STYLE_COUNT) return style_fill[styleidx].TextColor;
	return RGB(255, 255, 255);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// transparent BitBlt for win9x, TGA and BMP mode
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

BOOL trans_BitBlt(HDC hdcDst, int xd, int yd, int width, int height, HDC hdcSrc, int xs, int ys, int mode)
{
	BITMAPINFO bmpInfo = { { sizeof (BITMAPINFOHEADER) , width, 2*height, 1, 32, BI_RGB } };
	BYTE* pixels;
	HBITMAP bmpDIB = CreateDIBSection(NULL, &bmpInfo, DIB_RGB_COLORS, (PVOID*)&pixels, NULL, 0);
	if (NULL == bmpDIB) return FALSE;

	HDC hdcDIB = CreateCompatibleDC(hdcDst);
	HGDIOBJ otherDIB = SelectObject(hdcDIB, bmpDIB);
	BitBlt(hdcDIB, 0, height,   width, height, hdcDst, xd, yd, SRCCOPY);
	BitBlt(hdcDIB, 0, 0,        width, height, hdcSrc, xs, ys, SRCCOPY);

	BYTE *p = pixels;
	BYTE *e = p + width * height * 4;
	BYTE *s = e;

	if (TGATRANS == mode)
		while (p < e)
		{
			unsigned alpha_inv = 255 - s[3];
			p[2] = p[2] * alpha_inv / 255 + s[2];
			p[1] = p[1] * alpha_inv / 255 + s[1];
			p[0] = p[0] * alpha_inv / 255 + s[0];
			p += 4; s += 4;
		}
	else
	if (BMPTRANS == mode)
		while (p < e)
		{
			if (RGB(255, 0, 255) != *(COLORREF*)s)
				*(COLORREF*)p = *(COLORREF*)s;
			p += 4; s += 4;
		}

	BitBlt(hdcDst, xd, yd, width, height, hdcDIB,  0,  height, SRCCOPY);
	DeleteObject(SelectObject(hdcDIB, otherDIB));
	DeleteDC(hdcDIB);
	return TRUE;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//get_pressedstyle_index
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int get_pressedstyle_index(int styleidx){
	switch (styleidx)
	{
		case STYLETYPE_TOOLBAR:
			return STYLETYPE_INSET;
			break;
		case STYLETYPE_INSET:
		case STYLETYPE_LABEL:
		case STYLETYPE_CLOCK:
			return STYLETYPE_TOOLBAR; 
			break;
		case STYLETYPE_FLAT:
			return STYLETYPE_SUNKEN;
			break;
		case STYLETYPE_SUNKEN:
			return STYLETYPE_FLAT;
			break;
		case STYLETYPE_BUTTON:
			return STYLETYPE_PRESSEDBUTTON;
			break;
		case STYLETYPE_PRESSEDBUTTON:
			return STYLETYPE_BUTTON;
			break;

	}
	return STYLETYPE_NONE;
}

