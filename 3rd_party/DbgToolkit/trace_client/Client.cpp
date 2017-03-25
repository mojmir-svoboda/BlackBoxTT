#include "Client.h"
#include "trace.h"
#include <ScopeGuard.h>
#if !defined TRACE_CLIENT_DISABLE_NETWORKING
#	include "AsioSocketClient.h"
#endif
#include "utils_dbg.h"

template<class F, class...Ts, std::size_t...Is>
void for_each_in_tuple (F func, std::index_sequence<Is...>, std::tuple<Ts...> & tuple) {
	using expander = int[];
	(void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void for_each_in_tuple (F func, std::tuple<Ts...> & tuple) {
	for_each_in_tuple(func, std::make_index_sequence<sizeof...(Ts)>(), tuple);
}

namespace trace {

	void Client::Init (char const * appName)
	{
		SetAppName(appName);
		//std_apply([appName](auto & t) { t.SetAppName(appName); }, m_sinks);
		for_each_in_tuple([appName](auto & t) { t.SetAppName(appName); }, m_sinks);
	}

	void Client::SetLevelDictionary (level_t const * values, char const * names[], size_t sz)
	{
		for_each_in_tuple([values, names, sz] (auto & t) { t.SetLevelDictionary(values, names, sz); } , m_sinks);
	}
	void Client::SetContextDictionary (context_t const * values, char const * names[], size_t sz)
	{
		for_each_in_tuple([values, names, sz] (auto & t) { t.SetContextDictionary(values, names, sz); } , m_sinks);
	}

	void Client::Connect ()
	{
		DBG_OUT("Trace connecting sinks...");
		for_each_in_tuple([] (auto & t) { t.Connect(); } , m_sinks);
	}

	void Client::Disconnect ()
	{
		DBG_OUT("Trace disconnecting sinks...");
		for_each_in_tuple([] (auto & t) { t.Disconnect(); }, m_sinks);
	}

	void Client::Flush ()
	{
		DBG_OUT("Trace flushing...");
	}

	void Client::WriteMsg (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		//std::apply
		for_each_in_tuple(
			[level, context, file, line, fn, fmt, args] (auto & t)
			{
				t.WriteMsg(level, context, file, line, fn, fmt, args);
			}, m_sinks);
	}
// 	void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str);
	void Client::WriteScope (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		//std::apply
		for_each_in_tuple(
			[scptype, level, context, file, line, fn, fmt, args] (auto & t)
			{
				t.WriteScope(scptype, level, context, file, line, fn, fmt, args);
			}, m_sinks);
	}

	void Client::WritePlot (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
	}

	void Client::WritePlotMarker (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
	}

	void Client::WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
	}

	void Client::WritePlotClear (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::WriteTable (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
	}

	void Client::WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
	}

	void Client::WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
	}

	void Client::WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
	{
	}

	void Client::WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
	}

	void Client::WriteTableSetHHeader (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
	{
	}

	void Client::WriteTableClear (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::WriteGanttBgn (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::WriteGanttScopeBgn (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args)
	{
	}

	void Client::WriteGanttBgn (level_t level, context_t context)
	{
	}

	void Client::WriteGanttEnd (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::WriteGanttEnd (level_t level, context_t context)
	{
	}

	void Client::WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::WriteGanttFrameBgn (level_t level, context_t context)
	{
	}

	void Client::WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::WriteGanttFrameEnd (level_t level, context_t context)
	{
	}

	void Client::WriteGanttClear (level_t level, context_t context, char const * fmt, va_list args)
	{
	}

	void Client::ExportToCSV (char const * file)
	{
	}

	void Client::WriteSound (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args)
	{
	}
}

