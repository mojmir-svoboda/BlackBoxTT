#include <stdio.h>
#include <functional>
#include <sys/types.h>
#include <sysfn/time_query.h>
#include <sysfn/os.h>
#include <Command.h>
#include "trace.h"
#include <trace_proto/encoder.h>
#include <trace_proto/header.h>
#include <trace_proto/encode_config.h>
#include <trace_proto/encode_log.h>
#include <trace_proto/encode_plot.h>
#include <trace_proto/encode_plot_marker.h>
#include <trace_proto/encode_sound.h>

namespace trace {

	namespace socks {
		bool WriteToSocket(char const * buff, size_t ln);
	}

	level_t * GetRuntimeCfgData ();
	size_t GetRuntimeCfgSize ();

	template <size_t N, typename FnT>
	inline void WriteMessageWithHeader (FnT const & encode_body_fn)
	{
		if (GetRuntimeBuffering())
		{
			//char buffer[max_msg_sz];
			//size_t const n = fn(buffer, max_msg_sz);
		}
		else
		{
			char tmp[N];
			asn1::Header & hdr = asn1::encode_header(tmp, N);
			const size_t n = encode_body_fn(tmp + sizeof(asn1::Header), N - sizeof(asn1::Header));
			hdr.m_len = n;
			socks::WriteToSocket(tmp, n + sizeof(asn1::Header));
		}
	}


	// message logging
	void WriteLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		auto func = [=] (char * buff, size_t buff_ln)
		{
			const size_t n = asn1::encode_log(buff, buff_ln, ScopedLog::e_None, level, context, file, line, fn, fmt, args);
			return n;
		};
		WriteMessageWithHeader<16384>(std::ref(func));
	}
	void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		// @TODO
	}
	void WriteScopeVA (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		auto func = [=] (char * buff, size_t buff_ln)
		{
			const size_t n = asn1::encode_log(buff, buff_ln, scptype, level, context, file, line, fn, fmt, args);
			return n;
		};
		WriteMessageWithHeader<16384>(std::ref(func));
	}


	// Plotting
	void WritePlot_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		auto func = [=] (char * buff, size_t buff_ln)
		{
			asn1::Asn1StackAllocator a;
			const size_t n = asn1::encode_plot(&a, buff, buff_ln, level, context, x, y, fmt, args);
			return n;
		};
		WriteMessageWithHeader<1024>(std::ref(func));
	}
	void WritePlotMarker_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		auto func = [=] (char * buff, size_t buff_ln)
		{
			asn1::Asn1StackAllocator a;
			const size_t n = asn1::encode_plot_marker(&a, buff, buff_ln, level, context, x, y, fmt, args);
			return n;
		};
		WriteMessageWithHeader<1024>(std::ref(func));
	}
	void WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_plot(msg, level, context, x, y, z, fmt, args));
	}
	void WritePlotClear_impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_plot_clear(msg, level, context, fmt, args));
	}


	// Table data logging
	void WriteTable_impl (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table(msg, level, context, x, y, fmt, args));
	}
	void WriteTable_impl (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table(msg, level, context, x, y, c, fmt, args));
	}
	void WriteTable_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table(msg, level, context, x, y, fg, bg, fmt, args));
	}
	void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table_setup_color(msg, level, context, x, y, fg, fmt, args));
	}
	void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table_setup_color(msg, level, context, x, y, fg, bg, fmt, args));
	}
	void WriteTableSetHHeader_impl (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table_setup_hhdr(msg, level, context, x, name, fmt, args));
	}
	void WriteTableClear_impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		//ENCODE_BODY(encode_table_clear(msg, level, context, fmt, args));
	}


	// gantt write functions
	void WriteGanttBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_bgn(msg, level, context, fmt, args));
	}
	void WriteGanttScopeBgnVA_Impl (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_scope_bgn(msg, level, context, tag_buff, tag_max_sz, fmt, args));
	}
	void WriteGanttBgn_Impl (level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_bgn(msg, level, context));
	}
	void WriteGanttEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_end(msg, level, context, fmt, args));
	}
	void WriteGanttEnd_Impl (level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_end(msg, level, context));
	}
	void WriteGanttFrameBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_bgn(msg, level, context, fmt, args));
	}
	void WriteGanttFrameBgn_Impl(level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_bgn(msg, level, context));
	}
	void WriteGanttFrameEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_end(msg, level, context, fmt, args));
	}
	void WriteGanttFrameEnd_Impl (level_t level, context_t context)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_frame_end(msg, level, context));
	}
	void WriteGanttClearVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		// @TODO
		//ENCODE_BODY(encode_gantt_clear(msg, level, context, fmt, args));
	}

	void ExportToCSV (char const * file)
	{
		//ENCODE_BODY(encode_exportCSV(msg, file));
	}

	void WriteSound_impl (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args)
	{
		auto func = [=] (char * buff, size_t buff_ln)
		{
			asn1::Asn1StackAllocator a;
			const size_t n = asn1::encode_sound(&a, buff, buff_ln, level, context, vol, loop, fmt, args);
			return n;
		};
		WriteMessageWithHeader<1024>(std::ref(func));
	}
}

