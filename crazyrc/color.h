#pragma once
#include <wtypes.h>
#include "unicode.h"

struct Color {
	int r; int g; int b; int a;

	Color () : r(0), g(0), b(0), a(0) { }
	Color (int rr, int gg, int bb) : r(rr), g(gg), b(bb), a(0xFF) { }
	Color (int rr, int gg, int bb, int aa) : r(rr), g(gg), b(bb), a(aa) { }
	explicit Color (COLORREF const & cr) : r(GetRValue(cr)), g(GetGValue(cr)), b(GetBValue(cr)), a(0xFF) { }
};

inline tstream & operator<< (tstream & os, Color const & v)
{
  os << "rgb:" << v.r << "|" << v.g << "|" << v.b << "|" << v.a;
	return os;
}



