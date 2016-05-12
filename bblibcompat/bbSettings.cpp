/* ==========================================================================
This file is part of the bbLean source code
Copyright (C) 2001-2003 The Blackbox for Windows Development Team
Copyright (C) 2004-2009 grischka
========================================================================== */


#define BBSETTINGS_INTERNAL
#include "bbSettings.h"
#include "bblibcompat.h"
#include "utils_string.h"
#include "StyleStruct.h"
#include "styleprops.h"
#include "bbrc.h"
#include "iminmax.h"
#include "colors.h"
#include "utils_string.h"

#define BBSETTING_STYLEREADER_ONLY
#ifndef BBSETTING_STYLEREADER_ONLY
# include "Menu/MenuMaker.h"
# include "BImage.h"
# define BBSETTING
#endif
COLORREF SetContrast (int sn_index);

COLORREF get_bg_color(StyleItem *pSI)
{
	if (B_SOLID == pSI->type) // && false == pSI->interlaced)
		return pSI->Color;
	return mixcolors(pSI->Color, pSI->ColorTo, 128);
}

COLORREF get_mixed_color(StyleItem *pSI)
{
	COLORREF b = get_bg_color(pSI);
	COLORREF t = pSI->TextColor;
	if (greyvalue(b) > greyvalue(t))
		return mixcolors(t, b, 96);
	else
		return mixcolors(t, b, 144);
}



// to be used in multimonitor setups (in the future, maybe ...)
int screenNumber = 0;

/* *** Do not change this line. *** */
const int styleitem_size_assert = 1/(sizeof(StyleItem)==300?1:0);
/* If you get an error here that means that BBApi.h was changed
in an incompatible way */

//#define PARSEFONT_AFTER
//===========================================================================
// check a font if it is available on the system

static int CALLBACK EnumFontFamProc(
    ENUMLOGFONT FAR *lpelf,     // pointer to logical-font data
    NEWTEXTMETRIC FAR *lpntm,   // pointer to physical-font data
    int FontType,               // type of font
    LPARAM lParam               // address of application-defined data
    )
{
    (*(int*)lParam)++;
    return 0;
}

int checkfont (const wchar_t * face)
{
    int data = 0;
    HDC hdc = CreateCompatibleDC(NULL);
    EnumFontFamilies(hdc, face, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&data);
    DeleteDC(hdc);
    return data;
}

//===========================================================================
// convert weight into windows FW_XXX value
static int getweight (const wchar_t * p)
{
    static const wchar_t * const fontweightstrings[] = {
        TEXT("thin"), TEXT("extralight"), TEXT("light"), TEXT("normal"),
				TEXT("medium"), TEXT("demibold"), TEXT("bold"), TEXT("extrabold"),
				TEXT("heavy"), TEXT("regular"), TEXT("semibold"), NULL
    };
    int i = 1 + get_string_index(p, fontweightstrings);
    if (i>=10)
		{
        if (i==10) i = 4; // regular -> normal
        if (i==11) i = 6; // semibold -> demibold
    }
    return i*100;
}

static int tokenize_fontstring(char *buffer, char **pp, const char *src, int n, const char *delims)
{
//     const char *a, *s = src; int i = 0, r = 0, l;
//     while (i < n) {
//         l = nexttoken(&a, &s, delims);
//         *pp++ = extract_string(buffer, a, l);
//         buffer += l+1, i++, r += 0 != l;
//     }
//     return r;
}

void parse_font(StyleItem *si, const wchar_t *font)
{
//     static const wchar_t * const substlist[] = {
//         TEXT("lucidatypewriter")  ,   TEXT("edges")         ,
// 				TEXT("fixed")             ,   TEXT("lucida console"),
//         TEXT("lucida")            ,   TEXT("lucida sans")   ,
//         TEXT("helvetica")         ,   TEXT("tahoma")        ,
//         TEXT("calisto mt")        ,   TEXT("verdana")       ,
//         TEXT("8x8 system font")   ,   TEXT("edges")         ,
//         NULL
//     };
// 
//     wchar_t fontstring[100];
//     wchar_t *p[16];
//     wchar_t *b;
//     const wchar_t * const *r;
//     int i, w;
//     bool subst = false;
//     bool dblcheck = true;
// 
//     if ('-' == font[0])
//     {
//         /* --------------------------------------------------------
//         parse linux font spec:
//         -foundry-family-weight-slant-setwidth-addstyle
//         -pixel-point-resx-resy-spacing-width-charset-encoding
// 
//         slant: "-r-", "-i-", "-o-", "-ri-", "-ro-"
//         --------------------------------------------------------
//         */
//         enum {
//             f_foundry, f_family, f_weight, f_slant, f_setwidth, f_addstyle,
//             f_pixel, f_point, f_resx, f_resy, f_spacing, f_width, f_charset,
//             f_encoding, f_last
//         };
// 
//         tokenize_fontstring(fontstring, p, font+1, f_last, "-");
// 
//         if (*(b = p[f_family]) && *b != '*') {
//             strcpy(si->Font, b);
//             subst = true;
//         }
// 
//         if (*(b = p[f_weight])) {
//             si->FontWeight = 0 != (w = getweight(b)) ? w : FW_NORMAL;
//         }
// 
//         if (is_digit(*(b = p[f_pixel]))) { // 'pixel'
//             si->FontHeight = atoi(b);
//         }
//         else
//             if (is_digit(*(b = p[f_point]))) {// 'point'
//                 si->FontHeight = atoi(b) / 10;
//             }
//     } else if (checkfont(font)) {
//         // if the font exists, we take it as is
//         strcpy(si->Font, font);
//         dblcheck = false;
//     } else {
//         tokenize_fontstring(fontstring, p, font, 3,
//             strchr(font, '/') ? "/" :
//             strchr(font, ',') ? "," : "-");
//         for (i = 0; i < 3 && *(b = p[i]); ++i) {
//             if (0 == i) {
//                 strcpy(si->Font, b);
//                 subst = true;
//             } else if (is_digit(*b)) {
//                 si->FontHeight = atoi(b);
//             } else if (0 != (w = getweight(b))) {
//                 si->FontWeight = w;
//             } else if (stristr(b,"bold")) {
//                 si->FontWeight = FW_BOLD;
//             }
//         }
//     }
// 
//     if (subst)
//         for (r = substlist; r[0]; r+=2)
//             if (0 == _stricmp(si->Font, r[0])) {
//                 if (stristr(si->Font, "bold"))
//                     si->FontWeight = FW_BOLD;
//                 strcpy(si->Font, r[1]);
//                 break;
//             }
// 
//             //dbg_printf("%s -> <%s> %d %d", font, si->Font, si->FontHeight, si->FontWeight);
//             if (dblcheck && 0 == checkfont(si->Font)) {
//                 LOGFONT logFont;
//                 SystemParametersInfo(SPI_GETICONTITLELOGFONT, 0, &logFont, 0);
//                 strcpy(si->Font, logFont.lfFaceName);
//                 //si->FontHeight = logFont.lfHeight;
//                 {
//                     HFONT hf = CreateFontIndirect(&logFont);
//                     si->FontHeight = get_fontheight(hf);
//                     DeleteObject(hf);
//                 }
//             }
}

//===========================================================================
// API: CreateStyleFont
// Purpose: Create a Font, possible substitutions have been already applied.
//===========================================================================

HFONT CreateStyleFont(StyleItem const * pSI)
{
#ifdef PARSEFONT_AFTER
    StyleItem SI = *pSI;
    parse_font(&SI, pSI->Font);
    pSI = &SI;
#endif
    return CreateFont(
        pSI->FontHeight,
        0, 0, 0,
        pSI->FontWeight,
        false, false, false,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE,
        pSI->FontW
        );
}

//===========================================================================
int Settings_ItemSize(int sn_index)
{
    switch (sn_index) {
    case SN_STYLESTRUCT             : return sizeof (StyleStruct);

    case SN_TOOLBAR                 :
    case SN_TOOLBARBUTTON           :
    case SN_TOOLBARBUTTONP          :
    case SN_TOOLBARLABEL            :
    case SN_TOOLBARWINDOWLABEL      :
    case SN_TOOLBARCLOCK            :

    case SN_MENUTITLE               :
    case SN_MENUFRAME               :
    case SN_MENUGRIP                :
    case SN_MENUHILITE              : return sizeof (StyleItem);

    case SN_MENUBULLET              :
    case SN_MENUBULLETPOS           : return -1; // string, have to take strlen

    case SN_BORDERWIDTH             : return sizeof (int);
    case SN_BORDERCOLOR             : return sizeof (COLORREF);
    case SN_BEVELWIDTH              : return sizeof (int);
    case SN_FRAMEWIDTH              : return sizeof (int);
    case SN_HANDLEHEIGHT            : return sizeof (int);
    case SN_ROOTCOMMAND             : return -1; // string, have to take strlen

    case SN_MENUALPHA               : return sizeof (((StyleStruct*)0)->menuAlpha);
    case SN_TOOLBARALPHA            : return sizeof (((StyleStruct*)0)->toolbarAlpha);
    case SN_METRICSUNIX             :
    case SN_BULLETUNIX              : return sizeof (bool);

    case SN_WINFOCUS_TITLE          :
    case SN_WINFOCUS_LABEL          :
    case SN_WINFOCUS_HANDLE         :
    case SN_WINFOCUS_GRIP           :
    case SN_WINFOCUS_BUTTON         :
    case SN_WINFOCUS_BUTTONP        :
    case SN_WINUNFOCUS_TITLE        :
    case SN_WINUNFOCUS_LABEL        :
    case SN_WINUNFOCUS_HANDLE       :
    case SN_WINUNFOCUS_GRIP         :
    case SN_WINUNFOCUS_BUTTON       : return sizeof (StyleItem);

    case SN_WINFOCUS_FRAME_COLOR    :
    case SN_WINUNFOCUS_FRAME_COLOR  : return sizeof (COLORREF);

    case SN_ISSTYLE070              : return sizeof (bool);
    case SN_SLIT                    : return sizeof (StyleItem);

        /* BlackboxZero 1.8.2012 */
    case SN_MENUSEPMARGIN            : return sizeof (int);
    case SN_MENUSEPCOLOR            : return sizeof (COLORREF);
    case SN_MENUSEPSHADOWCOLOR        : return sizeof (COLORREF);

    default                         : return 0;
    }
}

//===========================================================================

static int ParseJustify (const wchar_t * buff)
{
    if (stristr(buff, L"center"))
        return DT_CENTER;
    if (stristr(buff, L"right"))
        return DT_RIGHT;
    return DT_LEFT;
}

static const wchar_t *check_global_font(const wchar_t *p, const wchar_t *fullkey)
{
#ifndef BBSETTING_STYLEREADER_ONLY
    if (Settings_globalFonts)
    {
        char globalkey[100];
        const char *p2;
        strcat(strcpy(globalkey, "blackbox.global."), fullkey);
        p2 = ReadValue(extensionsrcPath(NULL), globalkey, NULL);
        //dbg_printf("<%s> <%s>", globalkey, p2);
        if (p2 && p2[0]) return p2;
    }
#endif
    return p;
}

//===========================================================================
typedef struct ShortStyleItem
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
    char Font[16];
} ShortStyleItem;

static const ShortStyleItem DefStyle_1 = {
    BEVEL_RAISED, BEVEL1, B_DIAGONAL, false, false,
    0xEEEEEE, 0xCCCCCC, 0x555555, 12, FW_NORMAL, DT_LEFT, 0, ""
};

static const ShortStyleItem DefStyle_2 = {
    BEVEL_RAISED, BEVEL1, B_VERTICAL, false, false,
    0xCCCCCC, 0xAAAAAA, 0x333333, 12, FW_NORMAL, DT_LEFT, 0, ""
};

static const void *other_items [] =
{
    &DefStyle_1,
    &DefStyle_2,
    "",
    "right",
    "triangle"
};

enum other_defaults
{
    SN_DEFITEM_1 = SN_LAST,
    SN_DEFITEM_2,
    STR_EMPTY,
    STR_RIGHT,
    STR_TRIANGLE,
};

/* BlackboxZero 1.5.2012 - Added |V_OUTLINECOLOR|V_SHADOW where V_TXT  and |V_SPLIT where A_TEX*/
static const struct items StyleItems[] = {
    // bb4nix 065 style props --->>
    { C_INT, SN_BORDERWIDTH         , TEXT("borderWidth"),           0, 1 },
    { C_COL, SN_BORDERCOLOR         , TEXT("borderColor"),           0, 0 },
    { C_INT, SN_BEVELWIDTH          , TEXT("bevelWidth"),            0, 1 },
    { C_INT, SN_HANDLEHEIGHT        , TEXT("handleWidth"),           0, 5 },
    // -------------------------->>
    // The window frame from 065 is ignored since they are mostly set to random values in bb4win styles.
    //{ C_INT, SN_FRAMEWIDTH            , "frameWidth",                SN_BORDERWIDTH, 0 },
    //{ C_COL, SN_WINFOCUS_FRAME_COLOR  , "window.frame.focusColor",   SN_BORDERCOLOR, 0 },
    //{ C_COL, SN_WINUNFOCUS_FRAME_COLOR, "window.frame.unfocusColor", SN_BORDERCOLOR, 0 },
    // --------------------------<<
    { C_STR, SN_ROOTCOMMAND         , TEXT("rootCommand"),            STR_EMPTY       , sizeof mStyle.rootCommand },
    { C_STY, SN_TOOLBAR             , TEXT("toolbar"),                SN_DEFITEM_1    , A_TEX|V_MAR|V_TXT|A_FNT|I_DEF|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_TOOLBARLABEL        , TEXT("toolbar.label"),          SN_DEFITEM_1    , A_TEX|V_MAR|V_TXT|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_TOOLBARWINDOWLABEL  , TEXT("toolbar.windowLabel"),    SN_DEFITEM_2    , A_TEX|V_TXT|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_TOOLBARCLOCK        , TEXT("toolbar.clock"),          SN_DEFITEM_1    , A_TEX|V_TXT|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_TOOLBARBUTTON       , TEXT("toolbar.button"),         SN_DEFITEM_2    , A_TEX|V_PIC|V_MAR|V_SPLIT },
    { C_STY, SN_TOOLBARBUTTONP      , TEXT("toolbar.button.pressed"), SN_DEFITEM_1    , A_TEX|V_PIC|V_SPLIT },
#ifndef BBSETTING_NOMENU
    { C_STY, SN_MENUTITLE           , TEXT("menu.title"),             SN_DEFITEM_2    , A_TEX|V_MAR|V_TXT|A_FNT|I_DEF|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_MENUFRAME           , TEXT("menu.frame"),             SN_DEFITEM_1    , A_TEX|V_MAR|V_TXT|V_PIC|A_FNT|V_DIS|I_DEF|I_BUL|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_MENUHILITE          , TEXT("menu.active"),            SN_DEFITEM_2    , A_TEX|V_TXT|V_PIC|V_MAR|I_ACT|I_BUL|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STR, SN_MENUBULLET          , TEXT("menu.bullet"),            STR_TRIANGLE    , sizeof mStyle.menuBullet  },
    { C_STR, SN_MENUBULLETPOS       , TEXT("menu.bullet.position"),   STR_RIGHT       , sizeof mStyle.menuBulletPosition  },
#endif
#ifndef BBSETTING_NOWINDOW
    { C_STY, SN_WINFOCUS_TITLE      , TEXT("window.title.focus"),     SN_TOOLBAR      , A_TEX|I_DEF|V_SPLIT },
    { C_STY, SN_WINFOCUS_LABEL      , TEXT("window.label.focus"),     SN_TOOLBARWINDOWLABEL, A_TEX|V_TXT|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_WINFOCUS_HANDLE     , TEXT("window.handle.focus"),    SN_TOOLBAR      , A_TEX|I_DEF|V_SPLIT },
    { C_STY, SN_WINFOCUS_GRIP       , TEXT("window.grip.focus"),      SN_TOOLBARWINDOWLABEL, A_TEX|I_DEF|V_SPLIT },
    { C_STY, SN_WINFOCUS_BUTTON     , TEXT("window.button.focus"),    SN_TOOLBARBUTTON, A_TEX|V_PIC|V_SPLIT },
    { C_STY, SN_WINFOCUS_BUTTONP    , TEXT("window.button.pressed"),  SN_TOOLBARBUTTONP, A_TEX|V_PIC|V_SPLIT },

    { C_STY, SN_WINUNFOCUS_TITLE    , TEXT("window.title.unfocus"),   SN_TOOLBAR      , A_TEX|I_DEF|V_SPLIT },
    { C_STY, SN_WINUNFOCUS_LABEL    , TEXT("window.label.unfocus"),   SN_TOOLBAR      , A_TEX|V_TXT|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    { C_STY, SN_WINUNFOCUS_HANDLE   , TEXT("window.handle.unfocus"),  SN_TOOLBAR      , A_TEX|I_DEF|V_SPLIT },
    { C_STY, SN_WINUNFOCUS_GRIP     , TEXT("window.grip.unfocus"),    SN_TOOLBARLABEL , A_TEX|I_DEF|V_SPLIT },
    { C_STY, SN_WINUNFOCUS_BUTTON   , TEXT("window.button.unfocus"),  SN_TOOLBARBUTTON, A_TEX|V_PIC|V_SPLIT },
    // -------------------------->>
    // new bb4nix 070 style props
    { C_STY, SN_WINFOCUS_TITLE      , TEXT("window.title"),           SN_TOOLBAR      , V_MAR|I_DEF },
    { C_STY, SN_WINFOCUS_LABEL      , TEXT("window.label"),           SN_TOOLBARLABEL , V_MAR },
    { C_STY, SN_WINFOCUS_BUTTON     , TEXT("window.button"),          SN_TOOLBARBUTTON, V_MAR },

    { C_COL, SN_WINFOCUS_FRAME_COLOR, TEXT("window.frame.focus.borderColor"),     SN_BORDERCOLOR },
    { C_COL, SN_WINUNFOCUS_FRAME_COLOR, TEXT("window.frame.unfocus.borderColor"), SN_BORDERCOLOR },
    { C_INT, SN_FRAMEWIDTH          , TEXT("window.frame.borderWidth"),           SN_BORDERWIDTH },
    { C_INT, SN_HANDLEHEIGHT        , TEXT("window.handleHeight"),                SN_HANDLEHEIGHT, 0 },
    // --------------------------<<
    // window.font:
    { C_STY, SN_WINFOCUS_LABEL      , TEXT("window"),                 SN_TOOLBAR      , A_FNT },
#endif
    { C_STY, SN_SLIT                , TEXT("slit"),                   SN_TOOLBAR      , A_TEX|V_MAR|V_TXT|V_PIC|A_FNT|V_DIS|I_DEF|I_BUL|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
	{ C_STY, SN_MENUGRIP            , TEXT("menu.grip"),              SN_TOOLBAR      , A_TEX|V_MAR|V_TXT|V_PIC|A_FNT|V_DIS|I_DEF|I_BUL|V_OUTLINECOLOR|V_SHADOW|V_SPLIT },
    /* BlackboxZero 1.8.2012 */
    { C_INT, SN_MENUSEPMARGIN       , TEXT("menu.separator.margin"),            0, 0 },
    { C_COL, SN_MENUSEPCOLOR        , TEXT("menu.separator.color"),            0, 0 },
    { C_COL, SN_MENUSEPSHADOWCOLOR  , TEXT("menu.separator.shadowColor"),        0, 0 },

    { 0,0,NULL,0,0 }
};

//===========================================================================

const struct items *GetStyleItems(void)
{
    return StyleItems;
}

//===========================================================================

static int read_style_item (
    const wchar_t *style,
    StyleStruct * pStyle,
    const wchar_t * key,
    StyleItem *si,
    StyleItem *si_def,
    int sn,
    int f,
    bool is_070
    )
{
    static const struct s_prop {
        const wchar_t *k, *o;
        int mode, f; /* BlackboxZero 1.5.2012 - Was short */
    } s_prop[] = {
        // texture type
        { TEXT(".appearance"),      TEXT("")                , C_TEX , V_TEX },
        // colors, from, to, text, pics
        { TEXT(".color1"),          TEXT(".color")          , C_CO1 , V_CO1 },
        { TEXT(".color2"),          TEXT(".colorTo")        , C_CO2 , V_CO2 },
        { TEXT(".textColor"),       NULL              , C_TXT , V_TXT },
        { TEXT(".foregroundColor"), TEXT(".picColor")       , C_PIC , V_PIC },
        { TEXT(".disabledColor"),   TEXT(".disableColor")   , C_DIS , V_DIS },
        // font settings
        { TEXT(".font"),            NULL              , C_FON , V_FON },
        { TEXT(".fontHeight"),      NULL              , C_FHE , V_FHE },
        { TEXT(".fontWeight"),      NULL              , C_FWE , V_FWE },
        { TEXT(".alignment"),       TEXT(".justify")        , C_JUS , V_JUS },
        // borders & margins - _new in BBNix 0.70
        { TEXT(".borderWidth"),     NULL              , C_BOW , V_BOW },
        { TEXT(".borderColor"),     NULL              , C_BOC , V_BOC },
        { TEXT(".marginWidth"),     NULL              , C_MAR , V_MAR },
        /* BlackboxZero 1.4.2012 */
        { TEXT(".shadowX"),            NULL            , C_SHAX , V_SHADOWX },
        { TEXT(".shadowY"),            NULL            , C_SHAY , V_SHADOWY },
        { TEXT(".shadowColor"),        NULL            , C_CO5 , V_SHADOWCOLOR },
        { TEXT(".outlineColor"),        NULL            , C_CO6 , V_OUTLINECOLOR },
        { TEXT(".color.splitTo"), TEXT(".color1.splitTo")    , C_CO1ST , V_FROMSPLITTO },
        { TEXT(".colorTo.splitTo"), TEXT(".color2.splitTo") , C_CO2ST , V_TOSPLITTO },

    };

    const struct s_prop *cp = s_prop;
    COLORREF cr;
    int w, trans;
    const wchar_t *p;
    wchar_t fullkey[128];
    wchar_t * lastword;

    trans = set_translate_065(false);

restart:
    w = _tcslen(key);
    lastword = w + (wchar_t *)memcpy(fullkey, key, w * sizeof(wchar_t));
    si->nVersion = 4;
    do
    {
        if (cp->f & f)
        {
            if (C_CO1 == cp->mode
                && si->type == B_SOLID
                && false == si->interlaced)
                _tcscpy(lastword, L".backgroundColor");
            else
                _tcscpy(lastword, cp->k);

            p = ReadValue(style, fullkey, NULL);
            if (NULL == p && cp->o) {
                // try 0.65 key
                _tcscpy(lastword, cp->o);
                p = ReadValue(style, fullkey, NULL);
            }

            switch (cp->mode) {
                // --- texture ---
            case C_TEX:
                if (p) {
                    ParseItem(p, si);
                    si->bordered = NULL != stristr(p, L"border");
                    if (sn == SN_MENUTITLE) {
                        if (false == si->parentRelative && stristr(p, L"label"))
                            pStyle->menuTitleLabel = true;
                        if (stristr(p, L"hidden"))
                            si->parentRelative = pStyle->menuNoTitle = true;
                    }
                } else {
                    if (f & I_ACT && false == is_070) {
                        key = L"menu.hilite";
                        f &= ~I_ACT;
                        goto restart;
                    }
                    memcpy(si, si_def, offsetof(StyleItem, Color));
                }
                break;

                // --- colors ---
            case C_CO1:
                cr = ReadColorFromString(p);
                if (CLR_INVALID == cr)
                    cr = si_def->Color;
                si->Color = cr;
                break;

            case C_CO2:
                cr = ReadColorFromString(p);
                if (CLR_INVALID == cr)
                    cr = si_def->ColorTo;
                si->ColorTo = cr;
                break;

            case C_TXT:
                cr = ReadColorFromString(p);
                if (CLR_INVALID == cr)
                    cr = si_def ->TextColor;
                si->TextColor = cr;
                break;

            case C_DIS:
                cr = ReadColorFromString(p);
                if (CLR_INVALID == cr)
                    cr = get_mixed_color(si);
                si->disabledColor = cr;
                break;

            case C_PIC:
                if (NULL == p && false == is_070 && (f & I_BUL)) {
                    _tcscpy(lastword, L".bulletColor"); // xoblite menu bullets
                    p = ReadValue(style, fullkey, NULL);
                }

                cr = ReadColorFromString(p);
                if (f & V_TXT) {
                    if (CLR_INVALID == cr)
                        cr = si->TextColor;
                    si->foregroundColor = cr;
                } else {
                    if (CLR_INVALID == cr)
                        cr = si_def->TextColor;
                    si->TextColor = cr;
                }
                break;

                // --- Border ---
            case C_BOC:
                cr = ReadColorFromString(p);
                if (CLR_INVALID == cr)
                    cr = pStyle->borderColor;
                si->borderColor = cr;
                break;

            case C_BOW:
                // for backwards compatibility:
                if (false == is_070 && (f & I_DEF) && (p || pStyle->borderWidth))
                    si->bordered = true;

                // otherwise borderWidth is only valid with 'border' in texture
                if (false == si->bordered)
                    continue;

                si->borderWidth = p ? _ttoi(p) : pStyle->borderWidth;
                break;

                // --- Font ---
            case C_FON: // fontFace
                p = check_global_font(p, fullkey);
                _tcscpy(si->FontW, p ? p : si_def->FontW);
                break;

            case C_FHE: // fontHeight
                p = check_global_font(p, fullkey);
                si->FontHeight = p ? _ttoi(p) : si_def->FontHeight;
                break;

            case C_FWE: // fontWeight
                p = check_global_font(p, fullkey);
                si->FontWeight = p && 0 != (w = getweight(p)) ? w : FW_NORMAL;
#if !defined PARSEFONT_AFTER && !defined __BBSM__
                parse_font(si, si->FontW);
#endif
                break;

                // --- Alignment ---
            case C_JUS:
                si->Justify = p ? ParseJustify(p) : si_def->Justify;
                break;

                // --- Margins ---
            case C_MAR:
                if (p) {
                    if (sn != SN_MENUHILITE || found_last_value() == 1) {
                        si->marginWidth = _ttoi(p);
                        break;
                    }
                }

                /* BlackboxZero 1.5.2012 */
            case C_SHAX:
                si->ShadowX = p ? _ttoi(p) : 0;
                break;

            case C_SHAY:
                si->ShadowY = p ? _ttoi(p) : 0;
                break;

            case C_CO5:
                si->ShadowColor = ReadColorFromString(p);
                break;

            case C_CO6:
                si->OutlineColor = ReadColorFromString(p);
                break;

            case C_CO1ST:
                si->ColorSplitTo = ReadColorFromString(p);
                break;

            case C_CO2ST:
                si->ColorToSplitTo = ReadColorFromString(p);
                break;
                /* BlackboxZero 1.5.2012 */

                // --- default margins, a sensible issue ---
                switch (sn) {
                case SN_MENUTITLE:
                    if (is_070)
                        si->marginWidth = 2;
                    else
                        si->marginWidth = pStyle->bevelWidth + 1;
                    break;
                case SN_MENUFRAME:
                    if (is_070)
                        si->marginWidth = 1;
                    else
                        if (BEVEL_SUNKEN == si->bevelstyle
                            || BEVEL2 == si->bevelposition)
                            si->marginWidth = si->bevelposition;
                        else
                            if (pStyle->MenuHilite.borderWidth)
                                si->marginWidth = 1;
                            else
                                si->marginWidth = 0;
                    break;
                case SN_MENUHILITE:
                    if (is_070)
                        si->marginWidth = 2;
                    else
                        si->marginWidth = pStyle->bevelWidth+2;
                    break;
                case SN_TOOLBARBUTTON:
                    si->marginWidth = imax(0,
                        pStyle->ToolbarLabel.marginWidth - (false == is_070));
                    break;
                case SN_WINFOCUS_BUTTON:
                    si->marginWidth = imax(0,
                        pStyle->windowLabelFocus.marginWidth - (false == is_070));
                    break;
                default:
                    if (is_070 || 0 == (f & I_DEF))
                        si->marginWidth = 2;
                    else
                        si->marginWidth = pStyle->bevelWidth;
                    break;
                }
                break;
            }
            if (p)
                si->validated |= cp->f;
        }
    }
    while ((char*)++cp < (char*)s_prop + sizeof s_prop);

    set_translate_065(trans);
    return 0 != (si->validated & f);
}

//===========================================================================
void ReadStyle(const wchar_t * style, StyleStruct * pStyle)
{
    const struct items *s, *ptr;
    StyleItem *T, *F;
    bool bu = pStyle->bulletUnix;
    bool valid_bevelWidth;
    bool valid_borderWidth;
    bool valid_borderColor;

    valid_bevelWidth =
        valid_borderWidth =
        valid_borderColor = false;

    memset(pStyle, 0, sizeof *pStyle);
    pStyle->bulletUnix = bu;
    pStyle->metricsUnix = true;
    pStyle->is_070 = 0 != get_070(style);

#ifndef BBSETTING_STYLEREADER_ONLY
    pStyle->toolbarAlpha = Settings_toolbar.alphaEnabled ? Settings_toolbar.alphaValue : 255;
    pStyle->menuAlpha = Settings_menu.alphaEnabled ? Settings_menu.alphaValue : 255;
#endif

    s = StyleItems;
    do {
        const void *p_default;
        const wchar_t *p;
        COLORREF cr;
        void *v;
        int sn, dn, maxlen, type, trans;

        sn = s->sn;
        dn = s->sn_def;
        type = s->type;
        v = StyleStructPtr(sn, pStyle);

        if (dn == 0)
            p_default = &s->flags;
        else if (dn < SN_LAST)
            p_default = StyleStructPtr(dn, pStyle);
        else
            p_default = other_items[dn - SN_LAST];

        if (C_STY == type) {
            read_style_item (
                style,
                pStyle,
                s->rc_string,
                (StyleItem*)v,
                (StyleItem*)p_default,
                sn,
                s->flags,
                pStyle->is_070
                );
            continue;
        }

        trans = set_translate_065(false);
        p = ReadValue(style, s->rc_string, NULL);
        set_translate_065(trans);

        if (p) switch (sn) {
        case SN_BORDERWIDTH:
            valid_borderWidth = true;
            break;
        case SN_BEVELWIDTH:
            valid_bevelWidth = true;
            break;
        case SN_BORDERCOLOR:
            valid_borderColor = true;
            break;
        }

        switch (type) {
        case C_INT:
            *(int*)v = p ? _ttoi(p) : *(int*)p_default;
            break;

        case C_STR:
            maxlen = s->flags;
            strcpy_max((wchar_t *)v, p ? p : (const wchar_t *)p_default, maxlen);
            break;

        case C_COL:
            cr = ReadColorFromString(p);
            if (CLR_INVALID == cr)
                cr = *(COLORREF*)p_default;
            *(COLORREF*)v = cr;
            break;
        }

    } while ((++s)->sn);

    // ----------------------------------------------------
    // set some defaults for missing style settings
    if (pStyle->Toolbar.validated & V_TXT) {
        if (0==(pStyle->ToolbarLabel.validated & V_TXT))
            pStyle->ToolbarLabel.TextColor = pStyle->Toolbar.TextColor;

        if (0==(pStyle->ToolbarClock.validated & V_TXT))
            pStyle->ToolbarClock.TextColor = pStyle->Toolbar.TextColor;

        if (0==(pStyle->ToolbarWindowLabel.validated & V_TXT))
            pStyle->ToolbarWindowLabel.TextColor = pStyle->Toolbar.TextColor;
    } else {
        if (pStyle->ToolbarLabel.parentRelative)
            pStyle->Toolbar.TextColor = get_mixed_color(&pStyle->ToolbarLabel);
        else
            if (pStyle->ToolbarClock.parentRelative)
                pStyle->Toolbar.TextColor = get_mixed_color(&pStyle->ToolbarClock);
            else
                if (pStyle->ToolbarWindowLabel.parentRelative)
                    pStyle->Toolbar.TextColor = get_mixed_color(&pStyle->ToolbarWindowLabel);
                else
                    pStyle->Toolbar.TextColor = get_mixed_color(&pStyle->ToolbarLabel);
    }

    if (0==(pStyle->ToolbarButtonPressed.validated & V_PIC))
        pStyle->ToolbarButtonPressed.TextColor = pStyle->ToolbarButton.TextColor;

    if (0==(pStyle->Toolbar.validated & V_JUS))
        pStyle->Toolbar.Justify = DT_CENTER;

    if (0==(pStyle->ToolbarWindowLabel.validated & V_TEX))
        if (pStyle->ToolbarLabel.validated & V_TEX)
            pStyle->ToolbarWindowLabel = pStyle->ToolbarLabel;

    // Set a light grey background for no style at all
    if (0 == (pStyle->MenuFrame.validated & V_TEX)) {
        if (0 == pStyle->rootCommand[0])
            strcpy(pStyle->rootCommand, "bsetroot -mod 4 4 -fg grey55 -bg grey60");
    }

    // menuTitle font defaults to menuFrame
    T = &pStyle->MenuTitle;
    F = &pStyle->MenuFrame;
    if (0 == (T->validated & V_FON)) {
        strcpy(T->Font, F->Font);
        if (0 == (T->validated & V_FHE))
            T->FontHeight = F->FontHeight;
        if (0 == (T->validated & V_FWE))
            T->FontWeight = F->FontWeight;
    }

    if (pStyle->is_070) {
        StyleItem *si;
        for (si = &pStyle->windowTitleFocus;
            si <= &pStyle->windowButtonUnfocus; ++si)
            if (0 == (si->validated & V_BOC)) {
                if (si >= &pStyle->windowTitleUnfocus)
                    si->borderColor = pStyle->windowFrameUnfocusColor;
                else
                    si->borderColor = pStyle->windowFrameFocusColor;
            }

            // setup some default border/bevel to satisfy old plugins from new styles
            if (false == valid_bevelWidth)
                pStyle->bevelWidth = pStyle->Toolbar.marginWidth;
            if (false == valid_borderWidth)
                pStyle->borderWidth = pStyle->Toolbar.borderWidth;
            if (false == valid_borderColor)
                pStyle->borderColor = pStyle->Toolbar.borderColor;
    }

    /* BlackboxZero 1.6.2012 */
    // default SplitColor
    ptr = StyleItems;
    do {
        const void *p_default;
        //memset(pStyle, 0, sizeof *pStyle);
        p_default = StyleStructPtr(ptr->sn_def, pStyle);
        StyleItem* pSI = (StyleItem*)p_default;
        bool is_split = (pSI->type == B_SPLITVERTICAL) || (pSI->type == B_SPLITHORIZONTAL);
        if (ptr->flags & V_FROMSPLITTO){
            if(is_split && !(pSI->validated & V_FROMSPLITTO)){
                unsigned int r = GetRValue(pSI->Color);
                unsigned int g = GetGValue(pSI->Color);
                unsigned int b = GetBValue(pSI->Color);
                r = iminmax(r + (r>>2), 0, 255);
                g = iminmax(g + (g>>2), 0, 255);
                b = iminmax(b + (b>>2), 0, 255);
                pSI->ColorSplitTo = RGB(r, g, b);
                pSI->validated |= V_FROMSPLITTO;
            }
        }
        if (ptr->flags & V_TOSPLITTO){
            if(is_split && !(pSI->validated & V_TOSPLITTO)){
                unsigned int r = GetRValue(pSI->ColorTo);
                unsigned int g = GetGValue(pSI->ColorTo);
                unsigned int b = GetBValue(pSI->ColorTo);
                r = iminmax(r + (r>>4), 0, 255);
                g = iminmax(g + (g>>4), 0, 255);
                b = iminmax(b + (b>>4), 0, 255);
                pSI->ColorToSplitTo = RGB(r, g, b);
                pSI->validated |= V_TOSPLITTO;
            }
        }
    } while ((++ptr)->sn_def);
    /* BlackboxZero 1.6.2012 */
}

//===========================================================================
#ifndef BBSETTING_STYLEREADER_ONLY
//===========================================================================
// API: ReadStyleItem

int ReadStyleItem(
    const char* fileName,
    const char* szKey,
    StyleItem* pStyleItemOut,
    StyleItem* pStyleItemDefault
    )
{
    int ret;
    int f = A_TEX | A_FNT|V_TXT|V_DIS | V_PIC | V_MAR;

    if (NULL == pStyleItemDefault)
        pStyleItemDefault = (StyleItem*)&DefStyle_1;

    memset(pStyleItemOut, 0, sizeof *pStyleItemOut);
    ret = read_style_item(
        fileName,
        &mStyle,
        szKey,
        pStyleItemOut,
        pStyleItemDefault,
        0,
        f,
        0 != get_070(fileName)
        );
    return ret;
}

//===========================================================================
struct rccfg { const char *key; char mode; const void *p_default; const void *ptr; };

static const struct rccfg extrc_cfg[] = {

    { "blackbox.appearance.bullet.unix",       C_BOL, (void*)true,          &mStyle.bulletUnix },
    { "blackbox.appearance.arrow.unix",        C_BOL, (void*)false,         &Settings_arrowUnix },
    { "blackbox.appearance.cursor.usedefault", C_BOL, (void*)false,         &Settings_useDefCursor },

    { "blackbox.desktop.marginLeft",           C_INT, (void*)-1,            &Settings_desktopMargin.left },
    { "blackbox.desktop.marginRight",          C_INT, (void*)-1,            &Settings_desktopMargin.right },
    { "blackbox.desktop.marginTop",            C_INT, (void*)-1,            &Settings_desktopMargin.top },
    { "blackbox.desktop.marginBottom",         C_INT, (void*)-1,            &Settings_desktopMargin.bottom },

    { "blackbox.snap.toPlugins",               C_BOL, (void*)true,          &Settings_snapPlugins },
    { "blackbox.snap.padding",                 C_INT, (void*)2,             &Settings_snapPadding },
    { "blackbox.snap.threshold",               C_INT, (void*)7,             &Settings_snapThreshold },

    { "blackbox.background.enabled",           C_BOL, (void*)true,          &Settings_enableBackground  },
    { "blackbox.background.smartWallpaper",    C_BOL, (void*)true,          &Settings_smartWallpaper },

    { "blackbox.workspaces.followActive",      C_BOL, (void*)true,          &Settings_followActive },
    { "blackbox.workspaces.altMethod",         C_BOL, (void*)true,          &Settings_altMethod },
    { "blackbox.workspaces.styleXPFix",        C_BOL, (void*)false,         &Settings_styleXPFix },

    { "blackbox.options.disableTray",          C_BOL, (void*)false,         &Settings_disableTray },
    { "blackbox.options.disableDesk",          C_BOL, (void*)false,         &Settings_disableDesk },
    { "blackbox.options.disableMargins",       C_BOL, (void*)false,         &Settings_disableMargins },
    { "blackbox.options.disableVWM",           C_BOL, (void*)false,         &Settings_disableVWM },
    { "blackbox.options.disableDDE",           C_BOL, (void*)false,         &Settings_disableDDE },

    { "blackbox.options.desktopHook",          C_BOL, (void*)false,         &Settings_desktopHook },
    { "blackbox.options.hideExplorer",         C_BOL, (void*)true,          &Settings_hideExplorer  },
    { "blackbox.options.hideTaskbar",          C_BOL, (void*)true,          &Settings_hideExplorerTray },

    { "blackbox.options.shellContextMenu",     C_BOL, (void*)false,         &Settings_shellContextMenu },
    { "blackbox.options.UTF8Encoding",         C_BOL, (void*)false,         &Settings_UTF8Encoding },
    { "blackbox.options.OldTray",              C_BOL, (void*)false,         &Settings_OldTray },

    /* BlackboxZero 1.7.2012 */
    { "blackbox.menu.keepHilite:",              C_BOL, (void*)false,        &Settings_menuKeepHilite },
    { "blackbox.recent.menuFile:",              C_STR, (void*)"",           &Settings_recentMenu },
    { "blackbox.recent.itemKeepSize:",          C_INT, (void*)3,            &Settings_recentItemKeepSize },
    { "blackbox.recent.itemSortSize:",          C_INT, (void*)5,            &Settings_recentItemSortSize },
    { "blackbox.recent.withBeginEnd:",          C_BOL, (void*)true,         &Settings_recentBeginEnd },
    /* BlackboxZero 1.7.2012 */
    { "blackbox.menu.grip.enabled:",            C_BOL, (void*)true,         &Settings_menusGripEnabled },

    { "blackbox.global.fonts.enabled",         C_BOL, (void*)false,         &Settings_globalFonts },
    { "blackbox.editor",                       C_STR, (void*)"notepad.exe",  Settings_preferredEditor },

    // --------------------------------

    { NULL, 0, NULL, NULL }
};

//===========================================================================
static const struct rccfg bbrc_cfg[] = {
    { "#toolbar.enabled",          C_BOL, (void*)true,          &Settings_toolbar.enabled },
    { "#toolbar.placement",        C_STR, (void*)"TopCenter",    Settings_toolbar.placement },
    { "#toolbar.widthPercent",     C_INT, (void*)66,            &Settings_toolbar.widthPercent },
    { "#toolbar.onTop",            C_BOL, (void*)false,         &Settings_toolbar.onTop },
    { "#toolbar.autoHide",         C_BOL, (void*)false,         &Settings_toolbar.autoHide },
    { "#toolbar.pluginToggle",     C_BOL, (void*)true ,         &Settings_toolbar.pluginToggle },
    { "#toolbar.alpha.enabled",    C_BOL, (void*)false,         &Settings_toolbar.alphaEnabled },
    { "#toolbar.alpha.value",      C_INT, (void*)255,           &Settings_toolbar.alphaValue },

    { ".menu.position.x",          C_INT, (void*)100,           &Settings_menu.pos.x },
    { ".menu.position.y",          C_INT, (void*)100,           &Settings_menu.pos.y },
    { ".menu.popupDelay",          C_INT, (void*)80,            &Settings_menu.popupDelay },
    { ".menu.closeDelay",          C_INT, (void*)80,            &Settings_menu.closeDelay },/*BlackboxZero 1.3.2012 */
    { ".menu.mouseWheelFactor",    C_INT, (void*)3,             &Settings_menu.mouseWheelFactor },
    { ".menu.minWidth",            C_INT, (void*)50,            &Settings_menu.minWidth },/* BlackboxZero 12.17.2011 */
    { ".menu.maxWidth",            C_INT, (void*)240,           &Settings_menu.maxWidth },
    { ".menu.openDirection",       C_STR, (void*)"right",       &Settings_menu.openDirection },
    { ".menu.onTop",               C_BOL, (void*)false,         &Settings_menu.onTop },
    { ".menu.sticky",              C_BOL, (void*)true,          &Settings_menu.sticky },
    { ".menu.pluginToggle",        C_BOL, (void*)true,          &Settings_menu.pluginToggle },
    { ".menu.showBroams",          C_BOL, (void*)false,         &Settings_menu.showBroams },
    { ".menu.showHiddenFiles",     C_BOL, (void*)false,         &Settings_menu.showHiddenFiles },
    { ".menu.sortbyExtension",     C_BOL, (void*)false,         &Settings_menu.sortByExtension },
    { ".menu.drawSeparators",      C_BOL, (void*)true,          &Settings_menu.drawSeparators },
    { ".menu.snapWindow",          C_BOL, (void*)true,          &Settings_menu.snapWindow },
    { ".menu.dropShadows",         C_BOL, (void*)false,         &Settings_menu.dropShadows },
    { ".menu.alpha.enabled",       C_BOL, (void*)false,         &Settings_menu.alphaEnabled },
    { ".menu.alpha.value",         C_INT, (void*)255,           &Settings_menu.alphaValue },

    { ".menu.icon.size",           C_INT, (void*)16,            &Settings_menu.iconSize },
    { ".menu.icon.saturation",     C_INT, (void*)255,           &Settings_menu.iconSaturation },
    { ".menu.icon.hue",            C_INT, (void*)0,             &Settings_menu.iconHue },
    { ".menu.spacing",             C_INT, (void*)0,             &Settings_menu.spacing },
    { ".menu.bullet.enabled",      C_BOL, (void*)true,          &Settings_menu.bullet_enabled },
    { ".menu.scroller.position",   C_STR, (void*)"right",       &Settings_menu.scrollerPosition }, 
    { ".menu.scrollButton.hue",    C_INT, (void*)0,             &Settings_menu.scrollHue },

    { ".menu.separator.style",     C_STR, (void*)"gradient",    &Settings_menu.separatorStyle },
    { ".menu.separator.fullWidth", C_BOL, (void*)true,          &Settings_menu.separatorFullWidth },
    { ".menu.separator.compact",   C_BOL, (void*)true,          &Settings_menu.separatorCompact },

    { "#workspaces_wraparound",    C_BOL, (void*)true,          &Settings_workspaces_wraparound },
    { "#workspaces",               C_INT, (void*)3,             &Settings_workspaces },
    { "#workspacesX",              C_INT, (void*)3,             &Settings_workspacesX },
    { "#workspacesY",              C_INT, (void*)1,             &Settings_workspacesY },
    { "#workspaceNames",           C_STR, (void*)"alpha,beta,gamma", &Settings_workspaceNames },
    { "#strftimeFormat",           C_STR, (void*)"%I:%M %p",     Settings_toolbar.strftimeFormat },
    { "#fullMaximization",         C_BOL, (void*)false,         &Settings_fullMaximization },
    { "#focusModel",               C_STR, (void*)"ClickToFocus", Settings_focusModel },

    { ".imageDither",              C_INT, (void*)0,             &Settings_imageDither },
    { ".force.font.Shadows:",      C_BOL, (void*)false,         &Settings_globalShadows },
    { ".outlineText:",             C_BOL, (void*)false,         &Settings_outlineText },
    { ".opaqueMove",               C_BOL, (void*)true,          &Settings_opaqueMove },
    { ".autoRaiseDelay",           C_INT, (void*)250,           &Settings_autoRaiseDelay },


    /* *nix settings, not used here
    // ----------------------------------
    { ".changeWorkspaceWithMouseWheel", C_BOL, (void*)false,    &Settings_desktopWheel   },
    { "#edgeSnapThreshold",        C_INT, (void*)7,        &Settings_snapThreshold },

    { "#focusLastWindow",          C_BOL, (void*)false,    &Settings_focusLastWindow },
    { "#focusNewWindows",          C_BOL, (void*)false,    &Settings_focusNewWindows },
    { "#windowPlacement",          C_STR, (void*)"RowSmartPlacement", &Settings_windowPlacement },
    { "#colPlacementDirection",    C_STR, (void*)"TopToBottom", &Settings_colPlacementDirection },
    { "#rowPlacementDirection",    C_STR, (void*)"LeftToRight", &Settings_rowPlacementDirection },

    { ".colorsPerChannel",         C_INT, (void*)4,        &Settings_colorsPerChannel },
    { ".doubleClickInterval",      C_INT, (void*)250,      &Settings_dblClickInterval },
    { ".cacheLife",                C_INT, (void*)5,        &Settings_cacheLife },
    { ".cacheMax",                 C_INT, (void*)200,      &Settings_cacheMax },
    // ---------------------------------- */

    { NULL,0,NULL,NULL }
};

//===========================================================================
static const char * makekey(char *buff, const struct rccfg *cp)
{
    const char *k = cp->key;
    if (k[0]=='.')
        sprintf(buff, "session%s", k);
    else
        if (k[0]=='#')
            sprintf(buff, "session.screen%d.%s", screenNumber, k+1);
        else
            return k;
    return buff;
}

static void Settings_ReadSettings(const char *bbrc, const struct rccfg *cp)
{
    do {
        char keystr[100];
        const char *key = makekey(keystr, cp);
        switch (cp->mode)
        {
        case C_INT:
            *(int*)cp->ptr = ReadInt(bbrc, key, (int)(DWORD_PTR)cp->p_default);
            break;
        case C_BOL:
            *(bool*)cp->ptr = ReadBool (bbrc, key, 0 != (int)(DWORD_PTR)cp->p_default);
            break;
        case C_STR:
            strcpy((char*)cp->ptr, ReadString (bbrc, key, (char*)cp->p_default));
            break;
        }
    } while ((++cp)->key);
}

static bool Settings_WriteSetting(const char *bbrc, const struct rccfg *cp, const void *v)
{
    do if (NULL == v || cp->ptr == v)
    {
        char keystr[100];
        const char *key = makekey(keystr, cp);
        switch (cp->mode)
        {
        case C_INT:
            WriteInt (bbrc, key, *(int*) cp->ptr);
            break;
        case C_BOL:
            WriteBool (bbrc, key, *(bool*) cp->ptr);
            break;
        case C_STR:
            WriteString (bbrc, key, (char*) cp->ptr);
            break;
        }
        if (v) return true;
    } while ((++cp)->key);
    return false;
}

//===========================================================================

//===========================================================================

void Settings_WriteRCSetting(const void *v)
{
    Settings_WriteSetting(bbrcPath(NULL), bbrc_cfg, v)
        ||
        Settings_WriteSetting(extensionsrcPath(NULL), extrc_cfg, v);
}

void Settings_ReadRCSettings(void)
{
    const char *p, *extrc, *bbrc;

#ifdef __BBCORE__
    defaultrc_path[0] = 0;
    p = ReadString(extensionsrcPath(NULL), "blackbox.theme:", "");
    if (p[0] && 0 != _stricmp(p, "default")) {
        FindRCFile(defaultrc_path, p, NULL);
    }
#endif

    extrc = extensionsrcPath(NULL);
    bbrc = bbrcPath(NULL);
    Settings_ReadSettings(bbrc, bbrc_cfg);
    Settings_ReadSettings(extrc, extrc_cfg);
    p = ReadString(extrc, "blackbox.contextmenu.itemAdjust", "28/28");
    sscanf(p, "%d/%d", &Settings_contextMenuAdjust[0], &Settings_contextMenuAdjust[1]);

#ifdef __BBCORE__
    menuPath(ReadString(bbrc, "session.menuFile", NULL));
    stylePath(ReadString(bbrc, "session.styleFile", ""));
    plugrcPath(ReadString(bbrc, "session.pluginFile", NULL));

    p = ReadString(extrc, "blackbox.options.log", "");
    Settings_LogFlag = 0;
    if (*p) {
        if (stristr(p, "Shutdown"))
            Settings_LogFlag |= LOG_SHUTDOWN;
        if (stristr(p, "Startup"))
            Settings_LogFlag |= LOG_STARTUP;
        if (stristr(p, "Tray"))
            Settings_LogFlag |= LOG_TRAY;
        if (stristr(p, "Plugins"))
            Settings_LogFlag |= LOG_PLUGINS;
    }
#endif
}

void Settings_ReadStyleSettings(void)
{
    //DWORD t = GetTickCount(); for (int i=0; i < 1000; ++i)
    ReadStyle(stylePath(NULL), &mStyle);
    //dbg_printf("1000 styles read in %d ms", GetTickCount() - t);

    bimage_init(Settings_imageDither, mStyle.is_070);
}

//===========================================================================
// API: GetSettingPtr - retrieve a pointer to a setting var/struct
//===========================================================================

void* GetSettingPtr(int sn_index)
{
    return StyleStructPtr(sn_index, &mStyle);
}

//===========================================================================
#endif //ndef BBSETTING_STYLEREADER_ONLY
