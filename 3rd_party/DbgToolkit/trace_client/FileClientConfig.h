#pragma once
#include "trace.h"

#if defined TRACE_ENABLED
# include "MixerConfig.h"

namespace trace {
	struct FileClientConfig
	{
		char m_fileName[128] { "TraceClient.txt" };
		MixerConfig m_mixer;

		FileClientConfig () { }

		void SetFileName (char const * name) { strncpy(m_fileName, name, sizeof(m_fileName)/sizeof(*m_fileName)); }
		char const * GetFileName () const { return m_fileName; }

		void SetRuntimeLevelForContext (context_t ctx, level_t level) { m_mixer.SetRuntimeLevelForContext(ctx, level); }
		level_t GetRuntimeLevelForContextBit (context_t b) { return m_mixer.GetRuntimeLevelForContextBit(b); }
		//level_t * GetRuntimeCfgData () { return m_mixer.GetRuntimeCfgData(); }
		//size_t GetRuntimeCfgSize () const { return m_mixer.GetRuntimeCfgSize(); }
		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz) { m_mixer.SetLevelDictionary(values, names, sz); }
		void SetContextDictionary (context_t const * values, char const * names[], size_t sz) { m_mixer.SetContextDictionary(values, names, sz); }
	};
}
#else // tracing is NOT enabled
#endif
