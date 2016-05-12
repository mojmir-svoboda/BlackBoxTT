/* ==========================================================================

  This file is part of the bbLean source code
  Copyright © 2001-2003 The Blackbox for Windows Development Team
  Copyright © 2004-2009 grischka

  http://bb4win.sourceforge.net/bblean
  http://developer.berlios.de/projects/bblean

  bbLean is free software, released under the GNU General Public License
  (GPL version 2). For details see:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  ========================================================================== */

// #include <wtypes.h>
// 
// /* opaque structure */
// struct bimage;
// 
// struct StyleItem;
// 
// /* Initialize the gradient code */
// /* ---------------------------- */
// /*
//     dither:
//         do dithering on 16 bit displays
// 
//     is_070:
//         act with blackboxwm 0.70 convention, i.e.:
//         - do not switch color1/2 with sunken gradients
//         - use color2 with solid interlaced
// */
// 
// void bimage_init(int dither, bool is_070);
// 
// /* Low level functions */
// /* ------------------- */
// 
// /* create the gradient in memory */
// struct bimage *bimage_create(int width, int height, StyleItem const * si);
// 
// /* destroy everything */
// void bimage_destroy(struct bimage *bi);
// 
// /* get a pointer to the pixel memory */
// BYTE *bimage_getpixels(struct bimage *bi);
// 
// 
// /* High level functions */
// /* ------------------- */
// 
// /* paint the gradient on hDC */
// void MakeGradient(
//     HDC hDC,
//     RECT rect,
//     int type,
//     COLORREF Color,
//     COLORREF ColorTo,
//     bool interlaced,
//     int bevelstyle,
//     int bevelposition,
//     int bevelWidth,  /* not used, set to 0 */
//     COLORREF borderColour,
//     int borderWidth
//     );
// 
// void MakeGradientEx(HDC hDC, RECT rect, int type, COLORREF colour_from, COLORREF colour_to, 
// 					COLORREF colour_from_splitto, COLORREF colour_to_splitto, bool interlaced, int bevelStyle,
// 					int bevelPosition, int bevelWidth, COLORREF borderColor, int borderWidth);
// 
// /* paint a border on hDC */
// void CreateBorder(HDC hDC, RECT const * prect, COLORREF borderColour, int borderWidth);
// 
// /* paint the gradient on hDC from StyleStruct */
// void MakeStyleGradient(HDC hDC, RECT const * rp, StyleItem const * pSI, bool withBorder);
// 
// /* Create a HBITMAP and paint the gradient on it */
// HBITMAP MakeGradientBitmap(int width, int height, struct StyleItem *pSI);
