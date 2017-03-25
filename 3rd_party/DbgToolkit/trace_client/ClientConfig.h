#pragma once
#include "trace.h"

#if defined TRACE_ENABLED
#	include <stdarg.h>
#	include <array>
#	include <vector>
#	include <sysfn/time_query.h>
#	include <trace_proto/encoder.h>
#	include <trace_proto/header.h>
#	include <trace_proto/encode_config.h>
#	include <trace_proto/encode_log.h>
#	include <trace_proto/encode_dictionary.h>

namespace trace {

	using mixervalues_t = std::array<level_t, sizeof(context_t) * CHAR_BIT>;

	struct ClientConfig
	{
		bool m_buffered { false };
		char m_appName[128] { "TraceClient" };
		char m_hostName[256] { "127.0.0.1" };
		char m_hostPort[16] { "13127" };
		mixervalues_t m_mixer;
		using level_values_t = std::vector<level_t>;
		using level_names_t = std::vector<char const *>;
		level_values_t m_levelValuesDict;
		level_names_t m_levelNamesDict;
		using context_values_t = std::vector<context_t>;
		using context_names_t = std::vector<char const *>;
		context_values_t m_contextValuesDict;
		context_names_t m_contextNamesDict;

		ClientConfig () { }

		void SetAppName (char const * name) { strncpy(m_appName, name, sizeof(m_appName)/sizeof(*m_appName)); }
		char const * GetAppName () const { return m_appName; }

		void SetHostName (char const * addr) { strncpy(m_hostName, addr, sizeof(m_hostName)/sizeof(*m_hostName)); }
		char const * GetHostName () const { return m_hostName; }

		void SetHostPort (char const * port) { strncpy(m_hostPort, port, sizeof(m_hostPort)/sizeof(*m_hostPort));  }
		char const * GetHostPort () const { return m_hostPort; }

		void SetRuntimeLevelForContext (context_t ctx, level_t level)
		{
			for (unsigned b = 0; ctx; ++b, ctx >>= 1)
			{
				if (ctx & 1)
					m_mixer[b] = level;
			}
		}
		level_t GetRuntimeLevelForContextBit (context_t b)
		{
			if (b < m_mixer.size())
				return m_mixer[b];
			return 0;
		}
		level_t * GetRuntimeCfgData () { return m_mixer.data(); }
		size_t GetRuntimeCfgSize () const { return m_mixer.size(); }

		void SetRuntimeBuffering (bool buffered) { m_buffered = buffered; }
		bool GetRuntimeBuffering () const { return m_buffered; }
		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz)
		{
			ClientConfig::level_values_t tmp_values(values, values + sz);
			m_levelValuesDict = std::move(tmp_values);
			ClientConfig::level_names_t tmp_names(names, names + sz);
			m_levelNamesDict = std::move(tmp_names);
		}
		void SetContextDictionary (context_t const * values, char const * names[], size_t sz)
		{
			ClientConfig::context_values_t tmp_values(values, values + sz);
			m_contextValuesDict = std::move(tmp_values);
			ClientConfig::context_names_t tmp_names(names, names + sz);
			m_contextNamesDict = std::move(tmp_names);
		}
	};
}
#else // tracing is NOT enabled
	; // the perfect line
#endif
