#pragma once

// @TODO: move to lib
inline int imax (int a, int b) { return a > b ? a : b; }

inline int imin (int a, int b) { return a < b ? a : b; }

inline int iminmax (int a, int b, int c)
{
  if (a < b) a = b;
  if (a > c) a = c;
  return a;
}

inline void BitBltRect (HDC hdc_to, HDC hdc_from, RECT * r)
{
  BitBlt(
    hdc_to,
    r->left, r->top, r->right - r->left, r->bottom - r->top,
    hdc_from,
    r->left, r->top,
    SRCCOPY
    );
}
