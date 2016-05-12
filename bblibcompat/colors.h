#pragma once
#include <platform_win.h>

COLORREF rgb (unsigned r, unsigned g, unsigned b);
COLORREF switch_rgb (COLORREF c);
COLORREF mixcolors (COLORREF c1, COLORREF c2, int f);
COLORREF shadecolor (COLORREF c, int f);
unsigned greyvalue (COLORREF c);

/* ------------------------------------------------------------------------- */
/* Function: ParseLiteralColor */
/* Purpose: Parses a given literal color and returns the hex value */
COLORREF ParseLiteralColor (LPTSTR color);
/* ------------------------------------------------------------------------- */
/* Function: ReadColorFromString */
/* Purpose: parse a literal or hexadecimal color string */
COLORREF ReadColorFromString (TCHAR const * string);

