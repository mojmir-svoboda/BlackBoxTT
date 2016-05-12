#include "winutils.h"
#include "memory.h"
#include <tchar.h>
#include <cstdlib>

bool FileExists (LPTSTR szFileName)
{
	DWORD a = GetFileAttributes(szFileName);
	return (DWORD)-1 != a && 0 == (a & FILE_ATTRIBUTE_DIRECTORY);
}

void dbg_printf (TCHAR const * fmt, ...)
{
    TCHAR buffer[4096];
    va_list arg;
    va_start(arg, fmt);
	int const x = _vstprintf_s(buffer, 4096, fmt, arg);
	_tcscpy(buffer + x, TEXT("\n"));
    OutputDebugString(buffer);
}

void dbg_window (HWND hwnd, TCHAR const * fmt, ...)
{
    TCHAR buffer[4096];
    va_list arg;
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    int x = GetClassName(hwnd, buffer, 80);
	x += _stprintf_s(buffer + x, 4096 - x, TEXT(" hwnd:%lx pid:%ld "), (DWORD)(DWORD_PTR)hwnd, pid);
    va_start(arg, fmt);
	_vstprintf_s(buffer + x, 4096 - x, fmt, arg);
    _tcscat(buffer, TEXT("\n"));
    OutputDebugString(buffer);
}

int _load_imp (void * pp, const TCHAR * dll, const char * proc)
{
    HMODULE hm = GetModuleHandle(dll);
    if (NULL == hm)
        hm = LoadLibrary(dll);
    if (hm)
        *(FARPROC*)pp = GetProcAddress(hm, proc);
    return 0 != *(DWORD_PTR*)pp;
}

int load_imp (void *pp, const TCHAR * dll, const char *proc)
{
    if (0 == *(DWORD_PTR*)pp && !_load_imp(pp, dll, proc))
        *(DWORD_PTR*)pp = 1;
    return have_imp(*(void**)pp);
}

void BitBltRect (HDC hdc_to, HDC hdc_from, RECT const * r)
{
    BitBlt(
        hdc_to,
        r->left, r->top, r->right-r->left, r->bottom-r->top,
        hdc_from,
        r->left, r->top,
        SRCCOPY
        );
}

HWND GetRootWindow (HWND hwnd)
{
    HWND pw = 0;
    HWND dw = GetDesktopWindow();
    while (NULL != (pw = GetParent(hwnd)) && dw != pw)
        hwnd = pw;
    return hwnd;
}

int is_bbwindow (HWND hwnd)
{
    return GetWindowThreadProcessId(hwnd, NULL) == GetCurrentThreadId();
}

int get_fontheight (HFONT hFont)
{
    HDC hdc = CreateCompatibleDC(NULL);
    HGDIOBJ other = SelectObject(hdc, hFont);
    int ret = 12;
    TEXTMETRIC TXM;
    if (GetTextMetrics(hdc, &TXM))
        ret = TXM.tmHeight - TXM.tmExternalLeading;/*-TXM.tmInternalLeading;*/
    SelectObject(hdc, other);
    DeleteDC(hdc);
    return ret;
}

int get_filetime (TCHAR const * fn, FILETIME * ft)
{
    WIN32_FIND_DATA data_bb;
    HANDLE h = FindFirstFile(fn, &data_bb);
    if (INVALID_HANDLE_VALUE == h)
    {
        ft->dwLowDateTime = ft->dwHighDateTime = 0;
        return 0;
    }
    FindClose(h);
    *ft = data_bb.ftLastWriteTime;
    return 1;
}

int diff_filetime (TCHAR const * fn, FILETIME * ft0)
{
    FILETIME ft;
    get_filetime(fn, &ft);
    return CompareFileTime(&ft, ft0) != 0;
}

unsigned long getfileversion (TCHAR const * path)
{
    TCHAR temp[MAX_PATH];
    DWORD dwHandle = 0, result = 0;
    UINT bytes = GetFileVersionInfoSize(_tcscpy(temp, path), &dwHandle);
    if (bytes) {
        TCHAR * buffer = static_cast<TCHAR *>(m_alloc(bytes * sizeof(TCHAR)));
        if (GetFileVersionInfo(temp, 0, bytes, buffer))
        {
            void * value = 0;
            if (VerQueryValue(buffer, TEXT("\\"), &value, &bytes))
            {
                VS_FIXEDFILEINFO * vs = static_cast<VS_FIXEDFILEINFO *>(value);
                result = ((vs->dwFileVersionLS & 0xFF) >> 0)
                    | ((vs->dwFileVersionLS & 0xFF0000) >> 8)
                    | ((vs->dwFileVersionMS & 0xFF) << 16)
                    | ((vs->dwFileVersionMS & 0xFF0000) << 8)
                    ;
            }}
        m_free(buffer);
    }
    /* dbg_printf("version number of %s %08x", path, result); */
    return result;
}

TCHAR const * replace_environment_strings_alloc (TCHAR * & out, TCHAR const * src)
{
    out = NULL;
    if (0 == _tcschr(src, '%'))
        return src;
    int len = (int)_tcslen(src) + 512;
    for (;;)
    {
        TCHAR * buf = static_cast<TCHAR *>(m_alloc(len * sizeof(TCHAR)));
        int const r = ExpandEnvironmentStrings(src, buf, len);
        if (r && r <= len)
            return out = buf;
        m_free(buf);
        if (!r)
            return src;
        len = r;
    }
}

TCHAR * replace_environment_strings (TCHAR * src, int max_size)
{
    TCHAR * tmp = 0;
    replace_environment_strings_alloc(tmp, src);
    if (tmp) {
        _tcsncpy(src, tmp, max_size);
        m_free(tmp);
    }
    return src;
}

TCHAR * win_error (TCHAR * msg, int msgsize)
{
    if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msg, msgsize, NULL))
        msg[0] = 0;

    /* strip \r\n */
    TCHAR * p = _tcschr(msg, 0);
    while (p > msg && (TCHAR)p[-1] <= ' ')
        --p;
    *p = 0;
    return msg;
}

void ForceForegroundWindow (HWND theWin)
{
    DWORD ThreadID1, ThreadID2;
    HWND fw = GetForegroundWindow();
    if(theWin == fw)
        return; /* Nothing to do if already in foreground */
    ThreadID1 = GetWindowThreadProcessId(fw, NULL);
    ThreadID2 = GetCurrentThreadId();
    /* avoid attaching to a hanging message-queue */
    int const attach = ThreadID1 != ThreadID2 && 0 == is_frozen(fw);
    if (attach)
        AttachThreadInput(ThreadID1, ThreadID2, TRUE);
    SetForegroundWindow(theWin);
    if (attach)
        AttachThreadInput(ThreadID1, ThreadID2, FALSE);
}

void SetOnTop (HWND hwnd)
{
    if (IsWindow(hwnd) && IsWindowVisible(hwnd)
        && !(GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
    {
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING);
    }
}

int is_frozen (HWND hwnd)
{
    DWORD_PTR dwres;
    return 0 == SendMessageTimeout(hwnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG|SMTO_NORMAL, 300, &dwres);
}

HWND window_under_mouse ()
{
    POINT pt;
    GetCursorPos(&pt);
    return GetRootWindow(WindowFromPoint(pt));
}

int BBWait (int delay, unsigned nObj, HANDLE * pObj)
{
    DWORD r = 0;
    int quit = 0;

    DWORD const t_end = (delay > 0) ? GetTickCount() + delay : 0;
    do
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
                quit = 1;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        DWORD r = WAIT_TIMEOUT;
        DWORD t_wait = 0;
        if (delay > 0)
        {
            DWORD const tick = GetTickCount();
            if (tick >= t_end)
                break;
            t_wait = t_end - tick;
        }
        else if (nObj)
            t_wait = INFINITE;
        else
            break;
        r = MsgWaitForMultipleObjects(nObj, pObj, FALSE, t_wait, QS_ALLINPUT);
    } while (r == WAIT_OBJECT_0 + nObj);

    if (quit)
        PostQuitMessage(0);
    if (r == WAIT_TIMEOUT)
        return -1;
    if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + nObj)
        return r - WAIT_OBJECT_0;
    return -2;
}

void BBSleep (unsigned millisec)
{
    BBWait(millisec, 0, NULL);
}

int run_process (TCHAR const * cmd, TCHAR const * dir, int flags)
{
    TCHAR * buf = _tcsdup(cmd);
    STARTUPINFO si;
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    if (flags & RUN_HIDDEN) {
        si.wShowWindow = SW_HIDE;
        si.dwFlags = STARTF_USESHOWWINDOW;
    }
    PROCESS_INFORMATION pi;
    BOOL r = CreateProcess(NULL, buf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, dir, &si, &pi);
    free(buf);

    if (FALSE == r)
        return -1;

    DWORD retcode = 0;
    if (flags & RUN_WAIT)
    {
        BBWait(0, 1, &pi.hProcess);
        GetExitCodeProcess(pi.hProcess, (DWORD*)&retcode);
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return retcode;
}

/* ------------------------------------------------------------------------- */
