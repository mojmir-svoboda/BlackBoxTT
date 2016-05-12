#pragma once
#include "styleprops.h"
#include "StyleItem.h"

void ParseItem(const wchar_t * szItem, StyleItem * item)
{
	parse_item(szItem, item);
}

//#ifdef __cplusplus
//extern "C" {
//#endif

styleprop const styleprop_1[] = {
 { TEXT("solid")           ,B_SOLID           },
 { TEXT("horizontal")      ,B_HORIZONTAL      },
 { TEXT("splithorizontal") ,B_SPLITHORIZONTAL }, // horizontal is match .*horizontal
 { TEXT("blockhorizontal") ,B_BLOCKHORIZONTAL },
 { TEXT("mirrorhorizontal"),B_MIRRORHORIZONTAL},
 { TEXT("wavehorizontal")  ,B_WAVEHORIZONTAL  },
 { TEXT("vertical")        ,B_VERTICAL        },
 { TEXT("splitvertical")   ,B_SPLITVERTICAL   }, // vertical is match .*vertical
 { TEXT("blockvertical")   ,B_BLOCKVERTICAL   },
 { TEXT("mirrorvertical")  ,B_MIRRORVERTICAL  },
 { TEXT("wavevertical")    ,B_WAVEVERTICAL    },
 { TEXT("diagonal")        ,B_DIAGONAL        },
 { TEXT("crossdiagonal")   ,B_CROSSDIAGONAL   },
 { TEXT("pipecross")       ,B_PIPECROSS       },
 { TEXT("elliptic")        ,B_ELLIPTIC        },
 { TEXT("rectangle")       ,B_RECTANGLE       },
 { TEXT("pyramid")         ,B_PYRAMID         },
 { NULL              ,-1                }
 };

styleprop const styleprop_2[] = {
 { TEXT("flat")        ,BEVEL_FLAT     },
 { TEXT("raised")      ,BEVEL_RAISED   },
 { TEXT("sunken")      ,BEVEL_SUNKEN   },
 { NULL          ,-1             }
 };

styleprop const styleprop_3[] = {
 { TEXT("bevel1")      ,BEVEL1 },
 { TEXT("bevel2")      ,BEVEL2 },
 { TEXT("bevel3")      ,BEVEL2+1 },
 { NULL          ,-1     }
 };

/* ------------------------------------------------------------------------- */
// parse a given string and assigns settings to a StyleItem class

styleprop const * get_styleprop (int n)
{
    switch (n) {
        case 1: return styleprop_1;
        case 2: return styleprop_2;
        case 3: return styleprop_3;
        default : return NULL;
    }
}

int findtex (TCHAR const * p, int prop)
{
    styleprop const * s = get_styleprop(prop);
    do
        if (_tcsstr(p, s->key))
            break;
    while ((++s)->key);
    return s->val;
}
int find_exact (TCHAR const * p, int prop)
{
    styleprop const * s = get_styleprop(prop);
    do
        if (_tcscmp(p, s->key) == 0)
            break;
    while ((++s)->key);
    return s->val;
}

void parse_item (const wchar_t * szItem, StyleItem * item)
{
//     TCHAR buf[256];
//     TCHAR * ptr = &buf[0];
//     int t = -1;
//     TCHAR option[256];
//     bool found = false;
//     _tcslwr(_tcscpy(buf, szItem));
//     t = item->parentRelative = NULL != _tcsstr(buf,  TEXT("parentrelative"));
//     if (t) {
//         item->type = item->bevelstyle = item->bevelposition = item->interlaced = 0;
//         return;
//     }
// 
//     while (NextToken(option, &ptr, NULL))
//     {
//         int gt = -1;
//         if (_tcslen(option) == 0)
//             break;
//         gt = find_exact(option, 1);
//         if (gt != -1)
//         {
//           found = true;
//           item->type = gt;
//           break;
//         }
//     }
//     if (!found)
//     {
//       item->type = _tcsstr(buf,  TEXT("gradient")) ? B_DIAGONAL : B_SOLID;
//     }
// 
//     t = findtex(buf, 2);
//     item->bevelstyle = (-1 != t) ? t : BEVEL_RAISED;
// 
//     t = BEVEL_FLAT == item->bevelstyle ? 0 : findtex(buf, 3);
//     item->bevelposition = (-1 != t) ? t : BEVEL1;
// 
//     item->interlaced = NULL != _tcsstr(buf,  TEXT("interlaced"));
}

int find_in_propitem (styleprop const * props, int value)
{
    int i = 0;
    while (props[i].key != NULL)
    {
        if (props[i].val == value)
            return i;
        ++i;
    }
    return -1;
}

//#ifdef __cplusplus
//}
//#endif


