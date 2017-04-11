#include "trace.h"
#include "Client.h"
#include <tuple>
#include "utils_dbg.h"
#include "FailoverClient.h"
//#include "apply.h"

namespace trace {

	std::unique_ptr<Client> g_Client = nullptr;

	bool Init (char const * appName)
	{
		g_Client.reset(new Client);
		if (g_Client)
			g_Client->Init(appName);
		return g_Client.get() != nullptr;
	}

	// dispatch Init to sink at index N
	template <unsigned N>
	void sinkInitFn (unsigned arg_count, va_list args)
	{
		std::get<N>(g_Client->m_sinks).Init(arg_count, args);
	}
	template <size_t... Ns>
	void callInitForSink (unsigned sink_index, std::index_sequence<Ns...>, unsigned arg_count, va_list args)
	{
		using fn_t = void (*) (unsigned arg_count, va_list args);
		constexpr static fn_t const funcs[] = { &sinkInitFn<Ns>... };
		fn_t const & nth = funcs[sink_index];
		(*nth)(arg_count, args);
	}
	void callInitForSink (unsigned sink_index, unsigned arg_count, va_list args)
	{
		callInitForSink(sink_index, std::make_index_sequence<std::tuple_size<Client::sinks_t>::value> (), arg_count, args);
	}
	void InitSinkVA (unsigned sink, unsigned arg_count, va_list args)
	{
		callInitForSink(sink, arg_count, args);
	}

	void SetLevelDictionary (level_t const * values, char const * names[], size_t sz)
	{
		if (g_Client)
			g_Client->SetLevelDictionary(values, names, sz);
	}

	void SetContextDictionary (context_t const * values, char const * names[], size_t sz)
	{
		if (g_Client)
			g_Client->SetContextDictionary(values, names, sz);
	}

	// dispatch SetRuntimeLevelForContext to sink at index N
	template <size_t N>
	void sinkSetRuntimeLevelForContextFn (context_t ctx, level_t level)
	{
		std::get<N>(g_Client->m_sinks).SetRuntimeLevelForContext(ctx, level);
	}
	template <size_t... Ns>
	void SetRuntimeLevelForContext (unsigned sink_index, std::index_sequence<Ns...>, context_t ctx, level_t level)
	{
		using fn_t = void (*) (context_t ctx, level_t level);
		constexpr static fn_t const funcs[] = { &sinkSetRuntimeLevelForContextFn<Ns>... };
		fn_t const & nth = funcs[sink_index];
		(*nth)(ctx, level);
	}
	void SetRuntimeLevelForContext (unsigned sink, context_t ctx, level_t level)
	{
		SetRuntimeLevelForContext(sink, std::make_index_sequence<std::tuple_size<Client::sinks_t>::value>{ }, ctx, level);
	}

	// dispatch UnsetRuntimeLevelForContext to sink at index N
	template <size_t N>
	void sinkUnsetRuntimeLevelForContextFn (context_t ctx, level_t level)
	{
		std::get<N>(g_Client->m_sinks).UnsetRuntimeLevelForContext(ctx, level);
	}
	template <size_t... Ns>
	void UnsetRuntimeLevelForContext (unsigned sink_index, std::index_sequence<Ns...>, context_t ctx, level_t level)
	{
		using fn_t = void(*) (context_t ctx, level_t level);
		constexpr static fn_t const funcs[] = { &sinkUnsetRuntimeLevelForContextFn<Ns>... };
		fn_t const & nth = funcs[sink_index];
		(*nth)(ctx, level);
	}
	void UnsetRuntimeLevelForContext (unsigned sink, context_t ctx, level_t level)
	{
		UnsetRuntimeLevelForContext(sink, std::make_index_sequence<std::tuple_size<Client::sinks_t>::value>{ }, ctx, level);
	}

	// dispatch SetBuffered to sink at index N
	template <unsigned N>
	void sinkSetBuffered (bool on)
	{
		std::get<N>(g_Client->m_sinks).SetBuffered(on);
	}
	template <size_t... Ns>
	void SetBuffered (unsigned sink_index, std::index_sequence<Ns...>, bool on)
	{
		using fn_t = void(*) (bool on);
		constexpr static fn_t const funcs[] = { &sinkSetBuffered<Ns>... };
		fn_t const & nth = funcs[sink_index];
		(*nth)(on);
	}
	void SetBuffered (unsigned sink, bool on)
	{
		SetBuffered(sink, std::make_index_sequence<std::tuple_size<Client::sinks_t>::value>{ }, on);
	}

	void Connect ()
	{
		DBG_OUT("Trace connecting to sinks...\n");
		if (g_Client)
			g_Client->Connect();
	}

	void Disconnect ()
	{
		DBG_OUT("Trace disconnecting...");
		if (g_Client)
			g_Client->Disconnect();
	}

	void Done ()
	{
		DBG_OUT("Trace disconnecting...");
		if (g_Client)
			g_Client.reset();
	}

	void Flush ()
	{
		DBG_OUT("Trace flushing...");
		if (g_Client)
			g_Client->Flush();
	}

	// message logging
	void WriteMsgVA_impl (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteMsg(level, context, file, line, fn, fmt, args);
		else
			FailoverClient::WriteMsg(level, context, file, line, fn, fmt, args);
	}
	void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		// @TODO
	}
	void WriteScopeVA (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteScope(scptype, level, context, file, line, fn, fmt, args);
	}


	// Plotting
	void WritePlot_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WritePlot(level, context, x, y, fmt, args);
	}
	void WritePlotMarker_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WritePlotMarker(level, context, x, y, fmt, args);
	}
	void WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WritePlotXYZ(level, context, x, y, z, fmt, args);
	}
	void WritePlotClear_impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WritePlotClear(level, context, fmt, args);
	}


	// Table data logging
	void WriteTable_impl (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTable(level, context, x, y, fmt, args);
	}
	void WriteTable_impl (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTable(level, context, x, y, c, fmt, args);
	}
	void WriteTable_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTable(level, context, x, y, fg, bg, fmt, args);
	}
	void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTableSetColor(level, context, x, y, fg, fmt, args);
	}
	void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTableSetColor(level, context, x, y, fg, bg, fmt, args);
	}
	void WriteTableSetHHeader_impl (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTableSetHHeader(level, context, x, name, fmt, args);
	}
	void WriteTableClear_impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteTableClear(level, context, fmt, args);
	}


	// gantt write functions
	void WriteGanttBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteGanttBgn(level, context, fmt, args);
	}
	void WriteGanttScopeBgnVA_Impl (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args)
	{
		if (g_Client)
		g_Client->WriteGanttScopeBgn(level, context, tag_buff, tag_max_sz, fmt, args);
	}
	void WriteGanttBgn_Impl (level_t level, context_t context)
	{
		if (g_Client)
		g_Client->WriteGanttBgn(level, context);
	}
	void WriteGanttEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteGanttEnd(level, context, fmt, args);
	}
	void WriteGanttEnd_Impl (level_t level, context_t context)
	{
		if (g_Client)
			g_Client->WriteGanttEnd(level, context);
	}
	void WriteGanttFrameBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteGanttFrameBgn(level, context, fmt, args);
	}
	void WriteGanttFrameBgn_Impl(level_t level, context_t context)
	{
		if (g_Client)
			g_Client->WriteGanttFrameBgn(level, context);
	}
	void WriteGanttFrameEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteGanttFrameEnd(level, context, fmt, args);
	}
	void WriteGanttFrameEnd_Impl (level_t level, context_t context)
	{
		if (g_Client)
			g_Client->WriteGanttFrameEnd(level, context);
	}
	void WriteGanttClearVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteGanttClear(level, context, fmt, args);
	}

	void ExportToCSV (char const * file)
	{
		if (g_Client)
			g_Client->ExportToCSV(file);
	}

	void WriteSound_impl (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args)
	{
		if (g_Client)
			g_Client->WriteSound(level, context, vol, loop, fmt, args);
	}
}
