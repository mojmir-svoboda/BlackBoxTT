#include "trace.h"
#include <tuple>
#include <tmpl/tmpl_rename.h>
#include <tmpl/tmpl_typelist.h>

#if !defined TRACE_CLIENT_DISABLE_FILES
#	include "FileClient.h"
#endif
#if !defined TRACE_CLIENT_DISABLE_NETWORKING
#include "AsioSocketClient.h"
#endif

namespace trace {

	using ClientSinkConfig = typelist<TRACE_CLIENT_SINKS>;

	struct Client
	{
		using sinks_t = tmpl_rename<ClientSinkConfig, std::tuple>;
		sinks_t m_sinks;
		char m_appName[128] { "TraceClient" };
	
		void Init (char const * appName);
		void SetAppName (char const * name) { strncpy(m_appName, name, sizeof(m_appName)/sizeof(*m_appName)); }
		char const * GetAppName () const { return m_appName; }

		// sink functionality
		void Connect ();
		void Disconnect ();
		void Flush ();
		//template<int N>
		//auto GetSink () -> decltype(std::get<N>(m_tuple)) { return std::get<N>(m_tuple); }

		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz);
		void SetContextDictionary (context_t const * values, char const * names[], size_t sz);

// 		template<int N>
// 		void SetRuntimeLevelForContext (context_t ctx, level_t level);

		// tracing functionality
		void WriteMsg (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args);
// 	void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str);
		void WriteScope (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args);
		void WritePlot (level_t level, context_t context, float x, float y, char const * fmt, va_list args);
		void WritePlotMarker (level_t level, context_t context, float x, float y, char const * fmt, va_list args);
		void WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args);
		void WritePlotClear (level_t level, context_t context, char const * fmt, va_list args);
		void WriteTable (level_t level, context_t context, int x, int y, char const * fmt, va_list args);
		void WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args);
		void WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args);
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args);
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args);
		void WriteTableSetHHeader (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args);
		void WriteTableClear (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttBgn (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttScopeBgn (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args);
		void WriteGanttBgn (level_t level, context_t context);
		void WriteGanttEnd (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttEnd (level_t level, context_t context);
		void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttFrameBgn (level_t level, context_t context);
		void WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttFrameEnd (level_t level, context_t context);
		void WriteGanttClear (level_t level, context_t context, char const * fmt, va_list args);
		void ExportToCSV (char const * file);
		void WriteSound (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args);
	};
}
