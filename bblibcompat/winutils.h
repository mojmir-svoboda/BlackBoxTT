#pragma once
#include <platform_win.h>
//#include "bblibapi.h"

void dbg_printf (TCHAR const *fmt, ...);
bool FileExists(LPTSTR szFileName);
void dbg_window (HWND hwnd, TCHAR const * fmt, ...);
//void BitBltRect (HDC hdc_to, HDC hdc_from, RECT const * r);
HWND GetRootWindow (HWND hwnd);
int is_bbwindow (HWND hwnd);
int get_fontheight (HFONT hFont);
int get_filetime (const TCHAR * fn, FILETIME * ft);
int diff_filetime (const TCHAR * fn, FILETIME * ft0);
unsigned long getfileversion (const TCHAR * path);
TCHAR const * replace_environment_strings_alloc (TCHAR * & out, TCHAR const * src);
TCHAR * replace_environment_strings (TCHAR * src, int max_size);
TCHAR * win_error (TCHAR * msg, int msgsize);
void ForceForegroundWindow (HWND theWin);
void SetOnTop (HWND hwnd);
int is_frozen (HWND hwnd);
HWND window_under_mouse ();
/* ------------------------------------------------------------------------- */
/* Function: BBWait */
/* Purpose: wait for some obj and/or delay, dispatch messages in between */
/* ------------------------------------------------------------------------- */
int BBWait (int delay, unsigned nObj, HANDLE * pObj);
/* ------------------------------------------------------------------------- */
/* API: BBSleep */
/* Purpose: pause for the given delay while blackbox remains responsive */
/* ------------------------------------------------------------------------- */
void BBSleep (unsigned millisec);
/* ------------------------------------------------------------------------- */
/* Function: run_process */
/* Purpose: low level process spawn, optionally wait for completion */
/* ------------------------------------------------------------------------- */
#define RUN_SHOWERRORS  0
#define RUN_NOERRORS    1
#define RUN_WAIT        2
#define RUN_HIDDEN      4
#define RUN_NOARGS      8
#define RUN_NOSUBST    16
#define RUN_ISPIDL     32
#define RUN_WINDIR     64
int run_process (const TCHAR * cmd, const TCHAR * dir, int flags);

int load_imp (void * pp, const TCHAR * dll, const char * proc);
int _load_imp (void * pp, const TCHAR * dll, const char * proc);
inline bool have_imp (void const * pp) { return (DWORD_PTR)pp > 1; }

