#pragma once
#include <platform_win.h>

#if defined TRACE_ENABLED
#	include <3rd_party/logging/logging.h>
#else
#	include <3rd_party/logging/trace_dummy.h>
#endif

// inline void dbg_printf (wchar_t const * fmt, ...)
// {
//   wchar_t buffer[1024];
//   va_list arg;
//   int x;
//   va_start(arg, fmt);
//   x = _vsnwprintf_s(buffer, 1024, _TRUNCATE, fmt, arg);
// 	buffer[x] = L'\n';
// 	buffer[x + 1] = L'\0';
//   OutputDebugStringW(buffer);
// }
// 
// inline void dbg_printf (char const * fmt, ...)
// {
// 	char buffer[1024];
// 	va_list arg;
// 	int x;
// 	va_start(arg, fmt);
// 	x = _vsnprintf_s(buffer, 1024, _TRUNCATE, fmt, arg);
// 	buffer[x] = L'\n';
// 	buffer[x + 1] = L'\0';
// 	OutputDebugStringA(buffer);
// }

