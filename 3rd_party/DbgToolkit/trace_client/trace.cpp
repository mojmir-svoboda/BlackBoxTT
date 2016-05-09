#include "trace.h"
#include <stdarg.h>
#include <array>
#include <vector>
#include <sysfn/time_query.h>
#include <trace_proto/encoder.h>
#include <trace_proto/header.h>
#include <trace_proto/dictionary.h>
#include <trace_proto/encode_config.h>
#include <trace_proto/encode_log.h>
#include <trace_proto/encode_dictionary.h>

	namespace sys {
		hptimer_t g_Start = 0, g_Freq = 1000000;
	}

#if defined TRACE_ENABLED
	namespace trace {

		using mixervalues_t = std::array<level_t, sizeof(context_t) * CHAR_BIT>;

		struct ClientConfig
		{
			bool m_buffered;
			std::string  m_appName;
			std::string  m_hostName;
			std::string  m_hostPort;
			mixervalues_t m_mixer;
			std::vector<DictionaryPair> m_levelDict;
			std::vector<DictionaryPair> m_ctxDict;

			ClientConfig ()
				: m_appName("trace_client")
				, m_hostName("localhost")
				, m_hostPort("13127")
				, m_buffered(false)
				, m_mixer()
			{ }
		};
		ClientConfig g_Config;

		// setup and utils
		void SetAppName (char const * name) { g_Config.m_appName = name; }
		char const * GetAppName () { return g_Config.m_appName.c_str(); }

		void SetHostName (char const * addr) { g_Config.m_hostName = addr; }
		char const * GetHostName () { return g_Config.m_hostName.c_str(); }

		void SetHostPort (char const * port) { g_Config.m_hostPort = port; }
		char const * GetHostPort () { return g_Config.m_hostPort.c_str(); }

		void SetRuntimeLevelForContext (context_t ctx, level_t level)
		{
			for (unsigned b = 0; ctx; ++b, ctx >>= 1)
			{
				if (ctx & 1)
					g_Config.m_mixer[b] |= level;
			}
		}
		level_t GetRuntimeLevelForContext (context_t ctx)
		{
			for (unsigned b = 0; ctx; ++b, ctx >>= 1)
			{
				if (ctx & 1)
					return g_Config.m_mixer[b];
			}
			return 0;
		}

		bool RuntimeFilterPredicate (level_t level, context_t context)
		{
			for (unsigned b = 0; context; ++b, context >>= 1)
			{
				if (context & 1)
				{
					context_t const curr_level_in_ctx = g_Config.m_mixer[b];
					if (level & curr_level_in_ctx)
						return true;
				}
			}
			return false;
		}

		level_t * GetRuntimeCfgData () { return g_Config.m_mixer.data(); }
		size_t GetRuntimeCfgSize () { return g_Config.m_mixer.size(); }

		void SetRuntimeBuffering (bool buffered) { g_Config.m_buffered = buffered; }
		bool GetRuntimeBuffering () { return g_Config.m_buffered; }
		void SetLevelDictionary (DictionaryPair const * pairs, size_t sz)
		{
			std::vector<DictionaryPair> tmp(pairs, pairs + sz);
			g_Config.m_levelDict = std::move(tmp);
		}
		void SetContextDictionary (DictionaryPair const * pairs, size_t sz)
		{
			std::vector<DictionaryPair> tmp(pairs, pairs + sz);
			g_Config.m_ctxDict = std::move(tmp);
		}
	
		namespace socks {
			bool WriteToSocket (char const * buff, size_t ln);
		}

		void SendDictionary (int type, DictionaryPair const * dict_ptr, size_t dict_sz)
		{
			// send config message
			enum : size_t { max_msg_size = 8192 };
			char msg[max_msg_size];
			asn1::Header & hdr = asn1::encode_header(msg, max_msg_size);
			asn1::DictPair const * const asn1_dict_ptr = reinterpret_cast<asn1::DictPair const *>(dict_ptr); // cast between layout compatible classes
			if (const size_t n = asn1::encode_dictionary(msg + sizeof(asn1::Header), max_msg_size - sizeof(asn1::Header), type, asn1_dict_ptr, dict_sz))
			{
				hdr.m_len = n;
				socks::WriteToSocket(msg, n + sizeof(asn1::Header));
			}
		}

		// on connect (to server) callback
		void OnConnectionEstablished ()
		{
			OutputDebugStringA("LOG: connected, sending config to server\n");
			enum : size_t { max_msg_size = 1024 };
			char msg[max_msg_size];
			asn1::Header & hdr = asn1::encode_header(msg, max_msg_size);
			char const * mixer_ptr = reinterpret_cast<char const *>(g_Config.m_mixer.data());
			size_t const mixer_sz = g_Config.m_mixer.size() * sizeof(level_t);
			if (const size_t n = asn1::encode_config(msg + sizeof(asn1::Header), max_msg_size - sizeof(asn1::Header), GetAppName(), mixer_ptr, mixer_sz, g_Config.m_buffered, sys::get_pid()))
			{
				hdr.m_len = n;
				socks::WriteToSocket(msg, n + sizeof(asn1::Header));
			}

			if (size_t n = g_Config.m_levelDict.size())
				SendDictionary(0, &g_Config.m_levelDict[0], n);
			if (size_t n = g_Config.m_ctxDict.size())
				SendDictionary(1, &g_Config.m_ctxDict[0], n);
		}

		// on config received (from server) callback
		void OnConnectionConfigCommand (Command const & cmd)
		{
			bool buffered = cmd.choice.config.buffered == 1;
			char grr[256];
			_snprintf_s(grr, 256, "LOG: received config command: buff=%u\n", buffered);
			OutputDebugString(grr);

/*			SetRuntimeBuffering(buffered);*/ // %%#$#$#&#@^#!^#@%^#%^@#%^@#%^@#%^

			OCTET_STRING const & omixer = cmd.choice.config.mixer;
			char const * const ptr = reinterpret_cast<char const *>(omixer.buf);
			level_t const * levels = reinterpret_cast<level_t const *>(ptr);
			assert(g_Config.m_mixer.size() == omixer.size / sizeof(level_t));
			for (level_t & l : g_Config.m_mixer)
				l = *levels++;
		}

		// message logging
		void WriteLog (level_t level, context_t context, char const * file, int, char const *, char const *, va_list);
		void WriteVA (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteLog(level, context, file, line, fn, fmt, args);
		}
		void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteVA(level, context, file, line, fn, fmt, args);
			va_end(args);
		}

		inline void WriteScopeVA (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args);
		inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteScopeVA(type, level, context, file, line, fn, fmt, args);
			va_end(args);
		}
		ScopedLog::ScopedLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
			: m_level(level), m_context(context), m_file(file), m_line(line), m_fn(fn), m_start(sys::queryTime_us()), m_enabled(true)
		{
			if (m_enabled && RuntimeFilterPredicate(level, context))
			{
				va_list args;
				va_start(args, fmt);
				WriteScopeVA(e_Entry, level, context, file, line, fn, fmt, args);
				va_end(args);
			}
		}
		ScopedLog::ScopedLog (bool enabled, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
			: m_level(level), m_context(context), m_file(file), m_line(line), m_fn(fn), m_start(sys::queryTime_us()), m_enabled(enabled)
		{
			if (m_enabled && RuntimeFilterPredicate(level, context))
			{
				va_list args;
				va_start(args, fmt);
				WriteScopeVA(e_Entry, level, context, file, line, fn, fmt, args);
				va_end(args);
			}
		}

		ScopedLog::~ScopedLog ()
		{
			if (m_enabled && RuntimeFilterPredicate(m_level, m_context))
				WriteScope(e_Exit, m_level, m_context, m_file, m_line, m_fn, "dt=%llu", sys::queryTime_us() - m_start);
		}


		// plot-data logging
		inline void WritePlot_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args);
		void WritePlotVA (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WritePlot_impl(level, context, x, y, fmt, args);
		}
		void WritePlot (level_t level, context_t context, float x, float y, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WritePlotVA(level, context, x, y, fmt, args);
			va_end(args);
		}
		inline void WritePlotMarker_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args);
		void WritePlotMarkerVA (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WritePlotMarker_impl(level, context, x, y, fmt, args);
		}
		void WritePlotMarker (level_t level, context_t context, float x, float y, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WritePlotMarkerVA(level, context, x, y, fmt, args);
			va_end(args);
		}

		inline void WritePlotClear_impl (level_t level, context_t context, char const * fmt, va_list args);
		void WritePlotClearVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WritePlotClear_impl(level, context, fmt, args);
		}
		void WritePlotClear (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WritePlotClearVA(level, context, fmt, args);
			va_end(args);
		}


		// table-data logging
		inline void WriteTable_impl (level_t level, context_t context, int x, int y, char const * fmt, va_list args);
		void WriteTableVA (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTable_impl(level, context, x, y, fmt, args);
		}
		void WriteTable (level_t level, context_t context, int x, int y, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableVA(level, context, x, y, fmt, args);
			va_end(args);
		}

		inline void WriteTable_impl (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args);
		void WriteTableVA (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTable_impl(level, context, x, y, c, fmt, args);
		}
		void WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableVA(level, context, x, y, c, fmt, args);
			va_end(args);
		}

		inline void WriteTable_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args);
		void WriteTableVA (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTable_impl(level, context, x, y, fg, bg, fmt, args);
		}
		void WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableVA(level, context, x, y, fg, bg, fmt, args);
			va_end(args);
		}

		inline void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args);
		void WriteTableSetColorVA (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableSetColor_impl(level, context, x, y, fg, fmt, args);
		}
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableSetColorVA(level, context, x, y, fg, fmt, args);
			va_end(args);
		}

		inline void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args);
		void WriteTableSetColorVA (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableSetColor_impl(level, context, x, y, fg, bg, fmt, args);
		}
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableSetColorVA(level, context, x, y, fg, bg, fmt, args);
			va_end(args);
		}

		inline void WriteTableSetHHeader_impl (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args);
		void WriteTableSetHHeaderVA (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableSetHHeader_impl(level, context, x, name, fmt, args);
		}
		void WriteTableSetHHeader (level_t level, context_t context, int x,  char const * name, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableSetHHeaderVA(level, context, x, name, fmt, args);
			va_end(args);
		}
		inline void WriteTableClear_impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteTableClearVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableClear_impl(level, context, fmt, args);
		}
		void WriteTableClear (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableClearVA(level, context, fmt, args);
			va_end(args);
		}




		// gantt chart event logging
		void WriteGanttBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttBgnVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttBgnVA_Impl(level, context, fmt, args);
		}
		void WriteGanttBgn (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttBgnVA(level, context, fmt, args);
			va_end(args);
		}

		void WriteGanttBgn_Impl (level_t level, context_t context);
		void WriteGanttBgn (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttBgn_Impl(level, context);
		}

		void WriteGanttEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttEndVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttEndVA_Impl(level, context, fmt, args);
		}
		void WriteGanttEnd (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttEndVA(level, context, fmt, args);
			va_end(args);
		}

		void WriteGanttEnd_Impl (level_t level, context_t context);
		void WriteGanttEnd (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttEnd_Impl(level, context);
		}

		void WriteGanttFrameBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttFrameBgnVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameBgnVA_Impl(level, context, fmt, args);
		}
		void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttFrameBgnVA(level, context, fmt, args);
			va_end(args);
		}
		void WriteGanttFrameBgn_Impl(level_t level, context_t context);
		void WriteGanttFrameBgn (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameBgn_Impl(level, context);
		}

		void WriteGanttFrameEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttFrameEndVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameEndVA_Impl(level, context, fmt, args);
		}
		void WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttFrameEndVA(level, context, fmt, args);
			va_end(args);
		}
		void WriteGanttFrameEnd_Impl (level_t level, context_t context);
		void WriteGanttFrameEnd (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameEnd_Impl(level, context);
		}

		void WriteGanttClearVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttClearVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttClearVA_Impl(level, context, fmt, args);
		}
		void WriteGanttClear (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttClearVA(level, context, fmt, args);
			va_end(args);
		}

		void WriteGanttScopeBgnVA_Impl (level_t level, context_t context, char * tag_buff, size_t max_size, char const * fmt, va_list args);
		void WriteGanttScopeBgnVA (level_t level, context_t context, char * tag_buff, size_t max_size, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttScopeBgnVA_Impl(level, context, tag_buff, max_size, fmt, args);
		}
		ScopedGantt::ScopedGantt (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttScopeBgnVA(level, context, m_tag, 256, fmt, args);
			va_end(args);
		}

		ScopedGantt::~ScopedGantt ()
		{
			WriteGanttEnd(m_level, m_context, "%s", m_tag);
		}

		ScopedGanttFrame::ScopedGanttFrame (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttFrameBgnVA(level, context, fmt, args);
			va_end(args);
		}

		ScopedGanttFrame::~ScopedGanttFrame()
		{
			WriteGanttFrameEnd(m_level, m_context);
		}

		inline void WriteSound_impl (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args);
		void WriteSoundVA (level_t level, context_t context, float vol, int loop, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteSound_impl(level, context, vol, loop, fmt, args);
		}
		void WriteSound (level_t level, context_t context, float vol, int loop, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteSoundVA(level, context, vol, loop, fmt, args);
			va_end(args);
		}



	}

#else // tracing is NOT enabled
	; // the perfect line
#endif
