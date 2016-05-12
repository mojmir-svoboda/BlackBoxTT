#pragma once
//#include "bblibapi.h"
#include <wtypes.h>
/* =========================================================================== */
/* StyleItem */

typedef struct StyleItem
{
    /* 0.0.80 */
    int bevelstyle;
    int bevelposition;
    int type;
    bool parentRelative;
    bool interlaced;

    /* 0.0.90 */
    COLORREF Color;
    COLORREF ColorTo;
    COLORREF TextColor;
    int FontHeight;
    int FontWeight;
    int Justify;
    int validated;

    TCHAR Font[128];

    /* bbLean 1.16 */
    int nVersion;
    int marginWidth;
    int borderWidth;
    COLORREF borderColor;
    COLORREF foregroundColor;
    COLORREF disabledColor;
    bool bordered;
    bool FontShadow; /* xoblite */

    /* BlackboxZero 1.4.2012 */
    union
    {
        struct { char ShadowX; char ShadowY; };
        unsigned short ShadowXY;
    };
    COLORREF ShadowColor;
    COLORREF OutlineColor;
    COLORREF ColorSplitTo;
    COLORREF ColorToSplitTo;
    /* BlackboxZero 1.4.2012 */

    char reserved[82]; /* keep sizeof(StyleItem) = 300 */

} StyleItem;
