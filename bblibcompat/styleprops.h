#pragma once
#include <platform_win.h>
#include <tchar.h>
//#include "bblibapi.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

/* Gradient types */
#define B_HORIZONTAL 0
#define B_VERTICAL 1
#define B_DIAGONAL 2
#define B_CROSSDIAGONAL 3
#define B_PIPECROSS 4
#define B_ELLIPTIC 5
#define B_RECTANGLE 6
#define B_PYRAMID 7
#define B_SOLID 8
/* BlackboxZero 1.5.2012 */
//#define B_SPLIT_VERTICAL      9
//#define B_MIRROR_VERTICAL 11//10
//#define B_MIRROR_HORIZONTAL   10//11
//#define B_SPLIT_HORIZONTAL    12
#define B_SPLITVERTICAL     B_VERTICAL+100
#define B_MIRRORHORIZONTAL  B_HORIZONTAL+200
#define B_MIRRORVERTICAL    B_VERTICAL+200
#define B_SPLITHORIZONTAL   B_HORIZONTAL+100
#define B_WAVEHORIZONTAL    B_HORIZONTAL+300
#define B_WAVEVERTICAL      B_VERTICAL+300
#define B_BLOCKHORIZONTAL   B_HORIZONTAL+400
#define B_BLOCKVERTICAL     B_VERTICAL+400
/* BlackboxZero 1.5.2012 */

/* Bevelstyle */
#define BEVEL_FLAT 0
#define BEVEL_RAISED 1
#define BEVEL_SUNKEN 2

/* Bevelposition */
#define BEVEL1 1
#define BEVEL2 2

/* ------------------------------------------------------------------------- */
/* parse a StyleItem */

struct StyleItem;

/*API_EXPORT*/ void parse_item (const wchar_t * szItem, StyleItem * item);
/*API_EXPORT*/ int findtex (TCHAR const * p, int prop);
struct styleprop
{
    TCHAR const * key;
    int val;
};
/*API_EXPORT*/ styleprop const * get_styleprop (int prop);
/*API_EXPORT*/ int find_in_propitem (styleprop const * props, int value);

extern styleprop const styleprop_1[];
extern styleprop const styleprop_2[];
extern styleprop const styleprop_3[];

//#ifdef __cplusplus
//}
//#endif


