#pragma once
#if defined TRACE_ENABLED
# include "FileClientWriters.h"

namespace trace {

	struct FailoverClient
	{
		static void WriteMsg (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
		{
			enum { N = 1024 };
			char tmp[N];
			char * buff_ptr = tmp;
			size_t buff_sz = N;

			LogMsg l;
			size_t const n = l.Write(tmp, N - 2, fmt, args);
			if (n > 0 && n < N)
			{
				tmp[n] = '\n';
				tmp[n + 1] = '\0';
#if defined WIN32 || defined WIN64
				OutputDebugStringA(tmp);
#else
				fprintf(stderr, "%s", tmp);
#endif
			}
		}
// 	void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str);
		void WriteScope (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args) { }
		void WritePlot (level_t level, context_t context, float x, float y, char const * fmt, va_list args) { }
		void WritePlotMarker (level_t level, context_t context, float x, float y, char const * fmt, va_list args) { }
		void WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args) { }
		void WritePlotClear (level_t level, context_t context, char const * fmt, va_list args) { }
		void WriteTable (level_t level, context_t context, int x, int y, char const * fmt, va_list args) { }
		void WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args) { }
		void WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args) { }
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args) { }
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args) { }
		void WriteTableSetHHeader (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args) { }
		void WriteTableClear (level_t level, context_t context, char const * fmt, va_list args) { }
		void WriteGanttBgn (level_t level, context_t context, char const * fmt, va_list args) { }
		void WriteGanttScopeBgn (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args) { }
		void WriteGanttBgn (level_t level, context_t context) { }
		void WriteGanttEnd (level_t level, context_t context, char const * fmt, va_list args) { }
		void WriteGanttEnd (level_t level, context_t context) { }
		void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, va_list args) { }
		void WriteGanttFrameBgn (level_t level, context_t context) { }
		void WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, va_list args) { }
		void WriteGanttFrameEnd (level_t level, context_t context) { }
		void WriteGanttClear (level_t level, context_t context, char const * fmt, va_list args) { }
	};
}
#else // tracing is NOT enabled
#endif
