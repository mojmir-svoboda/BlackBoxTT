#pragma once
#include "trace.h"

#if defined TRACE_ENABLED
#	include "MixerConfig.h"

namespace trace {

	struct SocketClientConfig
	{
		bool m_buffered { false };
		MixerConfig m_mixer;
		char m_appName[128] { "TraceClient" };
		char m_hostName[256] { "127.0.0.1" };
		char m_hostPort[16] { "13127" };

		SocketClientConfig () { }

		void SetAppName (char const * name) { strncpy(m_appName, name, sizeof(m_appName)/sizeof(*m_appName)); }
		char const * GetAppName () const { return m_appName; }

		void SetHostName (char const * addr) { strncpy(m_hostName, addr, sizeof(m_hostName)/sizeof(*m_hostName)); }
		char const * GetHostName () const { return m_hostName; }

		void SetHostPort (char const * port) { strncpy(m_hostPort, port, sizeof(m_hostPort)/sizeof(*m_hostPort));  }
		char const * GetHostPort () const { return m_hostPort; }

		void SetRuntimeLevelForContext (context_t ctx, level_t level) { m_mixer.SetRuntimeLevelForContext(ctx, level); }
		void UnsetRuntimeLevelForContext (context_t ctx, level_t level) { m_mixer.UnsetRuntimeLevelForContext(ctx, level); }
		level_t GetRuntimeLevelForContextBit (context_t b) { return m_mixer.GetRuntimeLevelForContextBit(b); }
		//level_t * GetRuntimeCfgData () { return m_mixer.GetRuntimeCfgData(); }
		//size_t GetRuntimeCfgSize () const { return m_mixer.GetRuntimeCfgSize(); }
		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz) { m_mixer.SetLevelDictionary(values, names, sz); }
		void SetContextDictionary (context_t const * values, char const * names[], size_t sz) { m_mixer.SetContextDictionary(values, names, sz); }
	};
}
#else // tracing is NOT enabled
	; // the perfect line
#endif
