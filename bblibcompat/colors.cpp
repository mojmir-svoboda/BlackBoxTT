#include "colors.h"
#include "iminmax.h"
#include "utils.h"
#include "utils_string.h"
#include "utils_paths.h"
#include <platform_win.h>
#include <tchar.h>

COLORREF rgb (unsigned r, unsigned g, unsigned b)
{
    return RGB(r,g,b);
}

COLORREF switch_rgb (COLORREF c)
{
    return ((c & 0x0000ff) << 16) | (c & 0x00ff00) | ((c & 0xff0000) >> 16);
}

COLORREF mixcolors (COLORREF c1, COLORREF c2, int f)
{
    int n = 255 - f;

    unsigned const r = (GetRValue(c1) * f + GetRValue(c2) * n) / 255;
    unsigned const g = (GetGValue(c1) * f + GetGValue(c2) * n) / 255;
    unsigned const b = (GetBValue(c1) * f + GetBValue(c2) * n) / 255;
    return RGB(r, g, b);
}

COLORREF shadecolor (COLORREF c, int f)
{
    unsigned const r0 = (int)GetRValue(c) + f;
    unsigned const g0 = (int)GetGValue(c) + f;
    unsigned const b0 = (int)GetBValue(c) + f;
    unsigned const r1 = iminmax(r0, 0, 255);
    unsigned const g1 = iminmax(g0, 0, 255);
    unsigned const b1 = iminmax(b0, 0, 255);
    return RGB(r1, g1, b1);
}

unsigned greyvalue (COLORREF c)
{
    unsigned const r = GetRValue(c);
    unsigned const g = GetGValue(c);
    unsigned const b = GetBValue(c);
    return (r * 79 + g * 156 + b * 21) / 256;
}

/* X-Windows color names */
struct litcolor1
{
    TCHAR const * cname;
    COLORREF cref;
};
litcolor1 const litcolor1_ary[] = {
    { TEXT("aliceblue"),      RGB(240,248,255) },
    { TEXT("beige"),          RGB(245,245,220) },
    { TEXT("black"),          RGB(0,0,0) },
    { TEXT("blanchedalmond"), RGB(255,235,205) },
    { TEXT("blueviolet"),     RGB(138,43,226) },
    { TEXT("cornflowerblue"), RGB(100,149,237) },
    { TEXT("darkblue"),       RGB(0,0,139) },
    { TEXT("darkcyan"),       RGB(0,139,139) },
    { TEXT("darkgray"),       RGB(169,169,169) },
    { TEXT("darkgreen"),      RGB(0,100,0) },
    { TEXT("darkkhaki"),      RGB(189,183,107) },
    { TEXT("darkmagenta"),    RGB(139,0,139) },
    { TEXT("darkred"),        RGB(139,0,0) },
    { TEXT("darksalmon"),     RGB(233,150,122) },
    { TEXT("darkslateblue"),  RGB(72,61,139) },
    { TEXT("darkturquoise"),  RGB(0,206,209) },
    { TEXT("darkviolet"),     RGB(148,0,211) },
    { TEXT("dimgray"),        RGB(105,105,105) },
    { TEXT("floralwhite"),    RGB(255,250,240) },
    { TEXT("forestgreen"),    RGB(34,139,34) },
    { TEXT("gainsboro"),      RGB(220,220,220) },
    { TEXT("ghostwhite"),     RGB(248,248,255) },
    { TEXT("gray"),           RGB(190,190,190) },
    { TEXT("greenyellow"),    RGB(173,255,47) },
    { TEXT("lavender"),       RGB(230,230,250) },
    { TEXT("lawngreen"),      RGB(124,252,0) },
    { TEXT("lightcoral"),     RGB(240,128,128) },
    { TEXT("lightgoldenrodyellow"), RGB(250,250,210) },
    { TEXT("lightgray"),      RGB(211,211,211) },
    { TEXT("lightgreen"),     RGB(144,238,144) },
    { TEXT("lightseagreen"),  RGB(32,178,170) },
    { TEXT("lightslateblue"), RGB(132,112,255) },
    { TEXT("lightslategray"), RGB(119,136,153) },
    { TEXT("limegreen"),      RGB(50,205,50) },
    { TEXT("linen"),          RGB(250,240,230) },
    { TEXT("mediumaquamarine"), RGB(102,205,170) },
    { TEXT("mediumblue"),     RGB(0,0,205) },
    { TEXT("mediumseagreen"), RGB(60,179,113) },
    { TEXT("mediumslateblue"), RGB(123,104,238) },
    { TEXT("mediumspringgreen"), RGB(0,250,154) },
    { TEXT("mediumturquoise"), RGB(72,209,204) },
    { TEXT("mediumvioletred"), RGB(199,21,133) },
    { TEXT("midnightblue"),   RGB(25,25,112) },
    { TEXT("mintcream"),      RGB(245,255,250) },
    { TEXT("moccasin"),       RGB(255,228,181) },
    { TEXT("navy"),           RGB(0,0,128) },
    { TEXT("navyblue"),       RGB(0,0,128) },
    { TEXT("oldlace"),        RGB(253,245,230) },
    { TEXT("palegoldenrod"),  RGB(238,232,170) },
    { TEXT("papayawhip"),     RGB(255,239,213) },
    { TEXT("peru"),           RGB(205,133,63) },
    { TEXT("powderblue"),     RGB(176,224,230) },
    { TEXT("saddlebrown"),    RGB(139,69,19) },
    { TEXT("sandybrown"),     RGB(244,164,96) },
    { TEXT("violet"),         RGB(238,130,238) },
    { TEXT("white"),          RGB(255,255,255) },
    { TEXT("whitesmoke"),     RGB(245,245,245) },
    { TEXT("yellowgreen"),    RGB(154,205,50) },
    };

struct litcolor5
{
  TCHAR const * cname;
  COLORREF cref[5];
};
litcolor5 litcolor5_ary[] = {

    { TEXT("antiquewhite"), { RGB(250,235,215), RGB(255,239,219), RGB(238,223,204), RGB(205,192,176), RGB(139,131,120) }},
    { TEXT("aquamarine"), { RGB(127,255,212), RGB(127,255,212), RGB(118,238,198), RGB(102,205,170), RGB(69,139,116) }},
    { TEXT("azure"), { RGB(240,255,255), RGB(240,255,255), RGB(224,238,238), RGB(193,205,205), RGB(131,139,139) }},
    { TEXT("bisque"), { RGB(255,228,196), RGB(255,228,196), RGB(238,213,183), RGB(205,183,158), RGB(139,125,107) }},
    { TEXT("blue"), { RGB(0,0,255), RGB(0,0,255), RGB(0,0,238), RGB(0,0,205), RGB(0,0,139) }},
    { TEXT("brown"), { RGB(165,42,42), RGB(255,64,64), RGB(238,59,59), RGB(205,51,51), RGB(139,35,35) }},
    { TEXT("burlywood"), { RGB(222,184,135), RGB(255,211,155), RGB(238,197,145), RGB(205,170,125), RGB(139,115,85) }},
    { TEXT("cadetblue"), { RGB(95,158,160), RGB(152,245,255), RGB(142,229,238), RGB(122,197,205), RGB(83,134,139) }},
    { TEXT("chartreuse"), { RGB(127,255,0), RGB(127,255,0), RGB(118,238,0), RGB(102,205,0), RGB(69,139,0) }},
    { TEXT("chocolate"), { RGB(210,105,30), RGB(255,127,36), RGB(238,118,33), RGB(205,102,29), RGB(139,69,19) }},
    { TEXT("coral"), { RGB(255,127,80), RGB(255,114,86), RGB(238,106,80), RGB(205,91,69), RGB(139,62,47) }},
    { TEXT("cornsilk"), { RGB(255,248,220), RGB(255,248,220), RGB(238,232,205), RGB(205,200,177), RGB(139,136,120) }},
    { TEXT("cyan"), { RGB(0,255,255), RGB(0,255,255), RGB(0,238,238), RGB(0,205,205), RGB(0,139,139) }},
    { TEXT("darkgoldenrod"), { RGB(184,134,11), RGB(255,185,15), RGB(238,173,14), RGB(205,149,12), RGB(139,101,8) }},
    { TEXT("darkolivegreen"), { RGB(85,107,47), RGB(202,255,112), RGB(188,238,104), RGB(162,205,90), RGB(110,139,61) }},
    { TEXT("darkorange"), { RGB(255,140,0), RGB(255,127,0), RGB(238,118,0), RGB(205,102,0), RGB(139,69,0) }},
    { TEXT("darkorchid"), { RGB(153,50,204), RGB(191,62,255), RGB(178,58,238), RGB(154,50,205), RGB(104,34,139) }},
    { TEXT("darkseagreen"), { RGB(143,188,143), RGB(193,255,193), RGB(180,238,180), RGB(155,205,155), RGB(105,139,105) }},
    { TEXT("darkslategray"), { RGB(47,79,79), RGB(151,255,255), RGB(141,238,238), RGB(121,205,205), RGB(82,139,139) }},
    { TEXT("deeppink"), { RGB(255,20,147), RGB(255,20,147), RGB(238,18,137), RGB(205,16,118), RGB(139,10,80) }},
    { TEXT("deepskyblue"), { RGB(0,191,255), RGB(0,191,255), RGB(0,178,238), RGB(0,154,205), RGB(0,104,139) }},
    { TEXT("dodgerblue"), { RGB(30,144,255), RGB(30,144,255), RGB(28,134,238), RGB(24,116,205), RGB(16,78,139) }},
    { TEXT("firebrick"), { RGB(178,34,34), RGB(255,48,48), RGB(238,44,44), RGB(205,38,38), RGB(139,26,26) }},
    { TEXT("gold"), { RGB(255,215,0),  RGB(255,215,0), RGB(238,201,0), RGB(205,173,0), RGB(139,117,0) }},
    { TEXT("goldenrod"), { RGB(218,165,32), RGB(255,193,37), RGB(238,180,34), RGB(205,155,29), RGB(139,105,20) }},
    { TEXT("green"), { RGB(0,255,0), RGB(0,255,0), RGB(0,238,0), RGB(0,205,0), RGB(0,139,0) }},
    { TEXT("honeydew"), { RGB(240,255,240), RGB(240,255,240), RGB(224,238,224), RGB(193,205,193), RGB(131,139,131) }},
    { TEXT("hotpink"), { RGB(255,105,180), RGB(255,110,180), RGB(238,106,167), RGB(205,96,144), RGB(139,58,98) }},
    { TEXT("indianred"), { RGB(205,92,92), RGB(255,106,106), RGB(238,99,99), RGB(205,85,85), RGB(139,58,58) }},
    { TEXT("ivory"), { RGB(255,255,240), RGB(255,255,240), RGB(238,238,224), RGB(205,205,193), RGB(139,139,131) }},
    { TEXT("khaki"), { RGB(240,230,140), RGB(255,246,143), RGB(238,230,133), RGB(205,198,115), RGB(139,134,78) }},
    { TEXT("lavenderblush"), { RGB(255,240,245), RGB(255,240,245), RGB(238,224,229), RGB(205,193,197), RGB(139,131,134) }},
    { TEXT("lemonchiffon"), { RGB(255,250,205), RGB(255,250,205), RGB(238,233,191), RGB(205,201,165), RGB(139,137,112) }},
    { TEXT("lightblue"), { RGB(173,216,230), RGB(191,239,255), RGB(178,223,238), RGB(154,192,205), RGB(104,131,139) }},
    { TEXT("lightcyan"), { RGB(224,255,255), RGB(224,255,255), RGB(209,238,238), RGB(180,205,205), RGB(122,139,139) }},
    { TEXT("lightgoldenrod"), { RGB(238,221,130), RGB(255,236,139), RGB(238,220,130), RGB(205,190,112), RGB(139,129,76) }},
    { TEXT("lightpink"), { RGB(255,182,193), RGB(255,174,185), RGB(238,162,173), RGB(205,140,149), RGB(139,95,101) }},
    { TEXT("lightsalmon"), { RGB(255,160,122), RGB(255,160,122), RGB(238,149,114), RGB(205,129,98), RGB(139,87,66) }},
    { TEXT("lightskyblue"), { RGB(135,206,250), RGB(176,226,255), RGB(164,211,238), RGB(141,182,205), RGB(96,123,139) }},
    { TEXT("lightsteelblue"), { RGB(176,196,222), RGB(202,225,255), RGB(188,210,238), RGB(162,181,205), RGB(110,123,139) }},
    { TEXT("lightyellow"), { RGB(255,255,224), RGB(255,255,224), RGB(238,238,209), RGB(205,205,180), RGB(139,139,122) }},
    { TEXT("magenta"), { RGB(255,0,255), RGB(255,0,255), RGB(238,0,238), RGB(205,0,205), RGB(139,0,139) }},
    { TEXT("maroon"), { RGB(176,48,96), RGB(255,52,179), RGB(238,48,167), RGB(205,41,144), RGB(139,28,98) }},
    { TEXT("mediumorchid"), { RGB(186,85,211), RGB(224,102,255), RGB(209,95,238), RGB(180,82,205), RGB(122,55,139) }},
    { TEXT("mediumpurple"), { RGB(147,112,219), RGB(171,130,255), RGB(159,121,238), RGB(137,104,205), RGB(93,71,139) }},
    { TEXT("mistyrose"), { RGB(255,228,225), RGB(255,228,225), RGB(238,213,210), RGB(205,183,181), RGB(139,125,123) }},
    { TEXT("navajowhite"), { RGB(255,222,173), RGB(255,222,173), RGB(238,207,161), RGB(205,179,139), RGB(139,121,94) }},
    { TEXT("olivedrab"), { RGB(107,142,35), RGB(192,255,62), RGB(179,238,58), RGB(154,205,50), RGB(105,139,34) }},
    { TEXT("orange"), { RGB(255,165,0), RGB(255,165,0), RGB(238,154,0), RGB(205,133,0), RGB(139,90,0) }},
    { TEXT("orangered"), { RGB(255,69,0), RGB(255,69,0), RGB(238,64,0), RGB(205,55,0), RGB(139,37,0) }},
    { TEXT("orchid"), { RGB(218,112,214), RGB(255,131,250), RGB(238,122,233), RGB(205,105,201), RGB(139,71,137) }},
    { TEXT("palegreen"), { RGB(152,251,152), RGB(154,255,154), RGB(144,238,144), RGB(124,205,124), RGB(84,139,84) }},
    { TEXT("paleturquoise"), { RGB(175,238,238), RGB(187,255,255), RGB(174,238,238), RGB(150,205,205), RGB(102,139,139) }},
    { TEXT("palevioletred"), { RGB(219,112,147), RGB(255,130,171), RGB(238,121,159), RGB(205,104,137), RGB(139,71,93) }},
    { TEXT("peachpuff"), { RGB(255,218,185), RGB(255,218,185), RGB(238,203,173), RGB(205,175,149), RGB(139,119,101) }},
    { TEXT("pink"), { RGB(255,192,203), RGB(255,181,197), RGB(238,169,184), RGB(205,145,158), RGB(139,99,108) }},
    { TEXT("plum"), { RGB(221,160,221), RGB(255,187,255), RGB(238,174,238), RGB(205,150,205), RGB(139,102,139) }},
    { TEXT("purple"), { RGB(160,32,240), RGB(155,48,255), RGB(145,44,238), RGB(125,38,205), RGB(85,26,139) }},
    { TEXT("red"), { RGB(255,0,0), RGB(255,0,0), RGB(238,0,0), RGB(205,0,0), RGB(139,0,0) }},
    { TEXT("rosybrown"), { RGB(188,143,143), RGB(255,193,193), RGB(238,180,180), RGB(205,155,155), RGB(139,105,105) }},
    { TEXT("royalblue"), { RGB(65,105,225), RGB(72,118,255), RGB(67,110,238), RGB(58,95,205), RGB(39,64,139) }},
    { TEXT("salmon"), { RGB(250,128,114), RGB(255,140,105), RGB(238,130,98), RGB(205,112,84), RGB(139,76,57) }},
    { TEXT("seagreen"), { RGB(46,139,87), RGB(84,255,159), RGB(78,238,148), RGB(67,205,128), RGB(46,139,87) }},
    { TEXT("seashell"), { RGB(255,245,238), RGB(255,245,238), RGB(238,229,222), RGB(205,197,191), RGB(139,134,130) }},
    { TEXT("sienna"), { RGB(160,82,45), RGB(255,130,71), RGB(238,121,66), RGB(205,104,57), RGB(139,71,38) }},
    { TEXT("skyblue"), { RGB(135,206,235), RGB(135,206,255), RGB(126,192,238), RGB(108,166,205), RGB(74,112,139) }},
    { TEXT("slateblue"), { RGB(106,90,205), RGB(131,111,255), RGB(122,103,238), RGB(105,89,205), RGB(71,60,139) }},
    { TEXT("slategray"), { RGB(112,128,144), RGB(198,226,255), RGB(185,211,238), RGB(159,182,205), RGB(108,123,139) }},
    { TEXT("snow"), { RGB(255,250,250), RGB(255,250,250), RGB(238,233,233), RGB(205,201,201), RGB(139,137,137) }},
    { TEXT("springgreen"), { RGB(0,255,127), RGB(0,255,127), RGB(0,238,118), RGB(0,205,102), RGB(0,139,69) }},
    { TEXT("steelblue"), { RGB(70,130,180), RGB(99,184,255), RGB(92,172,238), RGB(79,148,205), RGB(54,100,139) }},
    { TEXT("tan"), { RGB(210,180,140), RGB(255,165,79), RGB(238,154,73), RGB(205,133,63), RGB(139,90,43) }},
    { TEXT("thistle"), { RGB(216,191,216), RGB(255,225,255), RGB(238,210,238), RGB(205,181,205), RGB(139,123,139) }},
    { TEXT("tomato"), { RGB(255,99,71), RGB(255,99,71), RGB(238,92,66), RGB(205,79,57), RGB(139,54,38) }},
    { TEXT("turquoise"), { RGB(64,224,208), RGB(0,245,255), RGB(0,229,238), RGB(0,197,205), RGB(0,134,139) }},
    { TEXT("violetred"), { RGB(208,32,144), RGB(255,62,150), RGB(238,58,140), RGB(205,50,120), RGB(139,34,82) }},
    { TEXT("wheat"), { RGB(245,222,179), RGB(255,231,186), RGB(238,216,174), RGB(205,186,150), RGB(139,126,102) }},
    { TEXT("yellow"), { RGB(255,255,0), RGB(255,255,0), RGB(238,238,0), RGB(205,205,0), RGB(139,139,0) }}
    };


COLORREF ParseLiteralColor (LPTSTR color)
{
    int i = 0, n = 0, s = 0;
    TCHAR * p = 0;
    TCHAR c = 0;
    TCHAR buf[64];
    TCHAR const * cp = 0;

    size_t l = _tcslen(color) + 1;
    if (l > array_count(buf))
        return (COLORREF) - 1;

    tmemcpy(buf, color, l);
    /*strlwr(buf); */

    while (NULL != (p = _tcschr(buf, ' ')))
    {
        _tcscpy(p, p + 1);
        --l;
    }

    if (l < 3)
        return (COLORREF) - 1;

    if (NULL != (p = _tcsstr(buf, TEXT("grey"))))
        p[2]='a';

    if (0 == tmemcmp(buf, TEXT("gray"), 4) && (c = buf[4]) >= '0' && c <= '9')
    {
        i = iminmax(_ttoi(buf + 4), 0, 100);
        i = (i * 255 + 50) / 100;
        return rgb(i,i,i);
    }

    i = *(p = &buf[l - 2]) - '0';
    if (i >= 1 && i <= 4)
        *p = 0, --l;
    else
        i = 0;

    cp = (TCHAR const *) litcolor5_ary;
    n = sizeof litcolor5_ary / sizeof litcolor5_ary[0];
    s = sizeof litcolor5_ary[0];
    for (;;) {
        do {
            if (*buf <= **(const TCHAR **)cp)
                break;
        } while (cp += s, --n);
        do {
            if (*buf < **(const TCHAR**)cp)
                break;
            if (0 == tmemcmp(buf, *(const TCHAR **)cp, l))
                return ((struct litcolor5*)cp)->cref[i];
        } while (cp += s, --n);

        if (i || s == sizeof litcolor1_ary[0])
            return (COLORREF)-1;

        cp = (TCHAR const *)litcolor1_ary;
        n = sizeof litcolor1_ary / sizeof litcolor1_ary[0];
        s = sizeof litcolor1_ary[0];
    }
}

COLORREF ReadColorFromString (TCHAR const * string)
{
    TCHAR stub[32];
		TCHAR stub2[32];
    TCHAR rgbstr[32];
    TCHAR *s, *d, *r, c;
    COLORREF cr;

    if (NULL == string)
        return CLR_INVALID;

		strcpy_max(stub, string, array_count(stub));
		unquote(stub2, stub);
    s = _tcslwr(stub2);

    /* check if its an "rgb:12/ee/4c" type string */
    if (0 == tmemcmp(s, TEXT("rgb:"), 4))
    {
        int j = 3;
        s +=4;
        d = rgbstr;
        r = s;
        for (;;)
        {
            d[0] = *r && '/'!=*r ? *r++ : '0';
            d[1] = *r && '/'!=*r ? *r++ : d[0];
            d += 2;
            if (0 == --j)
                break;
            if ('/' != *r)
                goto check_hex;
            ++r;
        }
        *d = 0;
        s = rgbstr;
    }
check_hex:
    /* check if its a valid hex number */
    if ('#'==*s)
        s++;
    for (cr = 0, d = s; (c = *d) != 0; ++d)
    {
        cr <<= 4;
        if (c >= '0' && c <= '9')
            cr |= c - '0';
        else
        if (c >= 'a' && c <= 'f')
            cr |= c - ('a' - 10);
        else /* must be a literal color name (or is invalid) */
            return ParseLiteralColor(s);
    }
    /* #AB4 short type colors */
    if (d - s == 3)
        cr = ((cr & 0xF00) << 12) | ((cr & 0xFF0) << 8) | ((cr & 0x0FF) << 4) | (cr & 0x00F);
    return switch_rgb(cr);
}

/* ------------------------------------------------------------------------- */
