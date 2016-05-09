/**
 * Copyright (C) 2011-2016 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once

#if !defined TRACE_ENABLED

# define TRACE_APPNAME(name)	((void)0)
#	define TRACE_SET_LEVEL(c,l)	((void)0)
#	define TRACE_SET_CONTEXT_MASK(c)	((void)0)
#	define TRACE_SET_LEVEL_DICTIONARY(dictpairs, size)	((void)0)
#	define TRACE_SET_CONTEXT_DICTIONARY(dictpairs, size)		((void)0)
#	define TRACE_CONNECT(addr, port)		((void)0)
#	define TRACE_DISCONNECT()	((void)0)
#	define TRACE_MSG(level, context, fmt, ... )		((void)0)
#	define TRACE_MSG_IF(condition, level, context, fmt, ... )		((void)0)
#	define TRACE_MSG_VA(level, context, fmt, va)	((void)0)
#	define TRACE_SCOPE(level, context)		((void)0)
#	define TRACE_SETBUFFERED(on)	((void)0)
#	define TRACE_CODE(code)		((void)0)
#	define TRACE_EXPORT_CSV(file)	((void)0)
#	define TRACE_PLOT_XY(...)	((void)0)
#	define TRACE_PLOT_XY_MARKER(...)	((void)0)
#	define TRACE_PLOT_XYZ(...)	((void)0)
#	define TRACE_SOUND(...)	((void)0)
#	define TRACE_TABLE(...)		((void)0)
#	define TRACE_TABLE(...)		((void)0)
#	define TRACE_TABLE_HHEADER(...)		((void)0)
#	define TRACE_TABLE_COLOR(...)	((void)0)
#	define TRACE_SCOPE_MSG(level, context, fmt, ...)	((void)0)
#	define TRACE_SCOPE_MSG_IF(condition, level, context, fmt, ...)	((void)0)
#	define TRACE_SCOPE_IF(condition, level, context)	((void)0)
#	define TRACE_SCOPE(level, context)	((void)0)
#	define TRACE_GANTT_BGN(fmt, ... )	((void)0)
#	define TRACE_GANTT_END(fmt, ... )	((void)0)
#	define TRACE_GANTT_BGN_VA(fmt, va)	((void)0)
#	define TRACE_GANTT_SCOPE(fmt, ...)	((void)0)
#	define TRACE_GANTT_FRAME_SCOPE(fmt, ...)	((void)0)
#	define TRACE_FLUSH()	((void)0)
#endif // !TRACE_ENABLED

