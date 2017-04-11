#pragma once
#if defined TRACE_ENABLED
# include "MixerConfig.h"
# include "File.h"
# include "FileClientWriters.h"
# include <tmpl/tmpl_typelist.h>
# include <tmpl/tmpl_if.h>
# include <tmpl/tmpl_rename.h>
# include <tmpl/tmpl_indexof.hpp>
# include <tmpl/tmpl_transform.h>
# include <tmpl/tmpl_at_c.h>

namespace trace {

	struct FileClientConfig
	{
		char m_fileName[128] { "TraceClient.txt" };
		char m_appName[128] { "TraceClient" };
		MixerConfig m_mixer;

		FileClientConfig () { }

		void SetAppName (char const * name) { strncpy(m_appName, name, sizeof(m_appName) / sizeof(*m_appName)); }
		void SetFileName (char const * name) { strncpy(m_fileName, name, sizeof(m_fileName)/sizeof(*m_fileName)); }
		char const * GetFileName () const { return m_fileName; }

		void SetRuntimeLevelForContext (context_t ctx, level_t level) { m_mixer.SetRuntimeLevelForContext(ctx, level); }
		void UnsetRuntimeLevelForContext (context_t ctx, level_t level) { m_mixer.UnsetRuntimeLevelForContext(ctx, level); }
		level_t GetRuntimeLevelForContextBit (context_t b) { return m_mixer.GetRuntimeLevelForContextBit(b); }
		//level_t * GetRuntimeCfgData () { return m_mixer.GetRuntimeCfgData(); }
		//size_t GetRuntimeCfgSize () const { return m_mixer.GetRuntimeCfgSize(); }
		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz) { m_mixer.SetLevelDictionary(values, names, sz); }
		void SetContextDictionary (context_t const * values, char const * names[], size_t sz) { m_mixer.SetContextDictionary(values, names, sz); }
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<int N>
	using meta_int = std::integral_constant<int, N>;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<class T>
	using argof = typename T::arg_t;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	using arguments_config_t = typelist<TRACE_ARGUMENT_ORDER>; /// supplied arguments from trace framework
	using user_args_t = tmpl_transform<argof, arguments_config_t>; /// mapping trace argument tags to native args
	using user_args_tuple_t = tmpl_rename<user_args_t, std::tuple>; /// tuple of trace user arguments

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<class L, class K>
	struct mp_map_find_impl;
	template<class L, class K>
	using mp_map_find = typename mp_map_find_impl<L, K>::type;

	template<template<class...> class L, class K>
	struct mp_map_find_impl<L<>, K>
	{
		using type = meta_int<-1>;
	};

	template<template<class...> class L, class T1, class... T, class K>
	struct mp_map_find_impl<L<T1, T...>, K>
	{
		using type = tmpl_if<std::is_base_of<T1, K>
			, tmpl_indexof<arguments_config_t, T1>
			, mp_map_find<L<T...>, K>>;
	};

	
	template<class T>
	struct FileClient
	{
		File m_file;
		FileClientConfig m_config;

		using writers_config_t = T; /// format of the log file

		template<typename T>
		using writer_to_arg_index = mp_map_find<arguments_config_t, T>;
		using writers_to_user_args_t = tmpl_transform<writer_to_arg_index, writers_config_t>; /// sequence of meta-integers mapping writer to argument index (or -1 if it does not exist)

		using writers_t = tmpl_rename<writers_config_t, std::tuple>;
		writers_t m_writers;
		LogMsg m_logMsg;

		template<int WriterIndex, int ArgIndex>
		struct wrap_writer
		{
			writers_t & m_writers;
			wrap_writer (writers_t & w) : m_writers(w) { }

			size_t operator() (char * buff, size_t buff_sz, user_args_tuple_t t)
			{
				size_t const written = std::get<WriterIndex>(m_writers).Write(buff, buff_sz, std::get<ArgIndex>(t));
				return written;
			}
		};

		template<size_t WriterIndex>
		size_t WriteNth (char * buff, size_t buff_sz, user_args_tuple_t t)
		{
			using idx_of_arg = typename tmpl_at_c<writers_to_user_args_t, WriterIndex>::type;
			wrap_writer<WriterIndex, idx_of_arg::value> w(m_writers); // @NOTE: partial specialization for <WriterIndex, -1> if no argument is assigned / required / found
			return w(buff, buff_sz, t);
		}

		template<size_t... Is>
		size_t WriteArgs (char * buff, size_t buff_sz, user_args_tuple_t t, std::index_sequence<Is...> )
		{
			using write_fn_prototype = size_t (FileClient::*) (char *, size_t, user_args_tuple_t);
			constexpr static write_fn_prototype const funcs[] = { &FileClient::WriteNth<Is>... };

			size_t written = 0;
			char * wrt_pos = buff;
			size_t avail = buff_sz;
			for (write_fn_prototype fn : funcs)
			{
				size_t const w = (this->*fn)(wrt_pos, avail, t);
				written += w;
				wrt_pos += w;
				if (avail < w)
				{
					avail = 0;
					return written;
				}
			}
			return written;
		}
		
		void WriteMsgImpl (user_args_tuple_t t, char const * fmt, va_list args)
		{
			char tmp[4096];
			size_t written = WriteArgs(tmp, 4096, t, std::make_index_sequence<std::tuple_size<writers_t>::value>{ });
			written += m_logMsg.Write(tmp + written, 4096 - written, fmt, args);
			m_file.Write(tmp, written);
			m_file.WriteEndl();
		}
		void WriteMsgImpl (user_args_tuple_t t, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteMsgImpl(t, fmt, args);
			va_end(args);
		}

		void SetAppName (char const * name) { return m_config.SetAppName(name); }

		void Init (int count, va_list args)
		{
			if (count > 0)
			{
				char const * arg0 = va_arg(args, char const *);
				m_config.SetFileName(arg0);
			}
		}
		void Connect ()
		{
			m_file.Open(m_config.m_fileName);
		}
		void Disconnect ()
		{
			m_file.Flush();
			m_file.Close();
		}
		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz)
		{
			m_config.SetLevelDictionary(values, names, sz);
		}
		void SetContextDictionary (level_t const * values, char const * names[], size_t sz)
		{
			m_config.SetContextDictionary(values, names, sz);
		}
		void SetRuntimeLevelForContext (context_t ctx, level_t level) { m_config.SetRuntimeLevelForContext(ctx, level); }
		void UnsetRuntimeLevelForContext (context_t ctx, level_t level) { m_config.UnsetRuntimeLevelForContext(ctx, level); }

		void SetBuffered (bool on) { m_file.SetBuffered(0); }

		// tracing functionality
		void WriteMsg (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
		{
			if (!m_config.m_mixer.RuntimeFilterPredicate(level, context))
				return;

			enum { N = 16384 };
			char tmp[N];
			char * buff_ptr = tmp;
			size_t buff_sz = N;

			WriteMsgImpl(std::make_tuple(level, context, file, line, fn), fmt, args);
		}
// 	void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str);
		void WriteScope (ScopedLog::E_Type scptype, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
		{
		}
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
		void ExportToCSV (char const * file) { }
	};

	// partial specialization for writers with no argument (like LogTime)
	template<class T>
	template<int N>
	struct FileClient<T>::wrap_writer<N, -1>
	{
		writers_t & m_writers;
		wrap_writer (writers_t & w) : m_writers(w) { }

		size_t operator() (char * buff, size_t buff_sz, user_args_tuple_t t)
		{
			size_t const written = std::get<N>(m_writers).Write(buff, buff_sz);
			return written;
		}
	};

}
#else // tracing is NOT enabled
#endif
