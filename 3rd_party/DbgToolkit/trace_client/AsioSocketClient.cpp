#include "trace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <trace_proto/header.h>
#include <trace_proto/decoder.h>
#define NOMINMAX 1
#include <asio.hpp>
#include <sysfn/time_query.h>
#include <ScopeGuard.h>
#include <trace_proto/header.h>
#include <trace_proto/encode_config.h>
#include <trace_proto/encode_log.h>
#include <trace_proto/encode_dictionary.h>

#define DBG_CLIENT 1
#if !defined DBG_CLIENT
#	define DBG_OUT(fmt, ...) ((void)0)
#else
#	include "utils_dbg.h"
#	define DBG_OUT(fmt, ...) dbg_out(fmt, __VA_ARGS__)
#endif

#include "AsioSocketClient.h"

namespace trace {

	ClientMemory g_ClientMemory;

	// message logging
	void AsioSocketClient::WriteMsg (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		enum { N = 16384 };
		char tmp[N];
		asn1::Header & hdr = asn1::encode_header(tmp, N);
		const size_t n = asn1::encode_log(tmp + sizeof(asn1::Header),  N - sizeof(asn1::Header), ScopedLog::e_None, level, context, file, line, fn, fmt, args);
		hdr.m_len = n;
		Write(tmp, n + sizeof(asn1::Header));
	}
// 	void AsioSocketClient::WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
// 	{
// 		// @TODO
// 	}
	void AsioSocketClient::WriteScope (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
// 			const size_t n = asn1::encode_log(buff, buff_ln, scptype, level, context, file, line, fn, fmt, args);
	}

	void AsioSocketClient::WritePlot (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
// 			asn1::Asn1StackAllocator a;
// 			const size_t n = asn1::encode_plot(&a, buff, buff_ln, level, context, x, y, fmt, args);
	}
	void AsioSocketClient::WritePlotMarker (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
// 			asn1::Asn1StackAllocator a;
// 			const size_t n = asn1::encode_plot_marker(&a, buff, buff_ln, level, context, x, y, fmt, args);
	}
	void AsioSocketClient::WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_plot(msg, level, context, x, y, z, fmt, args));
	}
	void AsioSocketClient::WritePlotClear (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_plot_clear(msg, level, context, fmt, args));
	}


	// Table data logging
	void AsioSocketClient::WriteTable (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
	}
	void AsioSocketClient::WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
	}
	void AsioSocketClient::WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
	}
	void AsioSocketClient::WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
	{
	}
	void AsioSocketClient::WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
	}
	void AsioSocketClient::WriteTableSetHHeader (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
	{
	}
	void AsioSocketClient::WriteTableClear (level_t level, context_t context, char const * fmt, va_list args)
	{
	}


	// gantt write functions
	void AsioSocketClient::WriteGanttBgn (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_bgn(msg, level, context, fmt, args));
	}
	void AsioSocketClient::WriteGanttScopeBgn (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_scope_bgn(msg, level, context, tag_buff, tag_max_sz, fmt, args));
	}
	void AsioSocketClient::WriteGanttBgn (level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_bgn(msg, level, context));
	}
	void AsioSocketClient::WriteGanttEnd (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_end(msg, level, context, fmt, args));
	}
	void AsioSocketClient::WriteGanttEnd (level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_end(msg, level, context));
	}
	void AsioSocketClient::WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_bgn(msg, level, context, fmt, args));
	}
	void AsioSocketClient::WriteGanttFrameBgn(level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_bgn(msg, level, context));
	}
	void AsioSocketClient::WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_end(msg, level, context, fmt, args));
	}
	void AsioSocketClient::WriteGanttFrameEnd (level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_end(msg, level, context));
	}
	void AsioSocketClient::WriteGanttClear (level_t level, context_t context, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_gantt_clear(msg, level, context, fmt, args));
	}

	void AsioSocketClient::ExportToCSV (char const * file)
	{
		//ENCODE_BODY(encode_exportCSV(msg, file));
	}

	void AsioSocketClient::WriteSound (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args)
	{
// 			asn1::Asn1StackAllocator a;
// 			const size_t n = asn1::encode_sound(&a, buff, buff_ln, level, context, vol, loop, fmt, args);
	}

}
