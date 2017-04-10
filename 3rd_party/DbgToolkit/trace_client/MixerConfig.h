#pragma once
#include "trace.h"

#if defined TRACE_ENABLED
#	include <stdarg.h>
#	include <array>
#	include <vector>

namespace trace {

	using mixervalues_t = std::array<level_t, sizeof(context_t) * CHAR_BIT>;

	struct MixerConfig
	{
		mixervalues_t m_mixer;
		using level_values_t = std::vector<level_t>;
		using level_names_t = std::vector<char const *>;
		level_values_t m_levelValuesDict;
		level_names_t m_levelNamesDict;
		using context_values_t = std::vector<context_t>;
		using context_names_t = std::vector<char const *>;
		context_values_t m_contextValuesDict;
		context_names_t m_contextNamesDict;

		MixerConfig () { }

		void SetRuntimeLevelForContext (context_t ctx, level_t level)
		{
			for (unsigned b = 0; ctx; ++b, ctx >>= 1)
			{
				if (ctx & 1)
					m_mixer[b] |= level;
			}
		}
		void UnsetRuntimeLevelForContext (context_t ctx, level_t level)
		{
			for (unsigned b = 0; ctx; ++b, ctx >>= 1)
			{
				if (ctx & 1)
					m_mixer[b] &= ~level;
			}
		}
		level_t GetRuntimeLevelForContextBit (context_t b)
		{
			if (b < m_mixer.size())
				return m_mixer[b];
			return 0;
		}
		level_t * GetMixerData () { return m_mixer.data(); }
		size_t GetMixerSize () const { return m_mixer.size(); }

		void SetLevelDictionary (level_t const * values, char const * names[], size_t sz)
		{
			level_values_t tmp_values(values, values + sz);
			m_levelValuesDict = std::move(tmp_values);
			level_names_t tmp_names(names, names + sz);
			m_levelNamesDict = std::move(tmp_names);
		}
		void SetContextDictionary (context_t const * values, char const * names[], size_t sz)
		{
			context_values_t tmp_values(values, values + sz);
			m_contextValuesDict = std::move(tmp_values);
			context_names_t tmp_names(names, names + sz);
			m_contextNamesDict = std::move(tmp_names);
		}

		/**@fn		RuntimeFilterPredicate
		* @brief	decides if message will be logged or not
		*/
		bool RuntimeFilterPredicate (level_t level, context_t context)
		{
			context_t ctx = context;
			for (unsigned b = 0; ctx; ++b, ctx >>= 1)
			{
				if (ctx & 1)
				{
					const context_t curr_level_in_ctx = GetRuntimeLevelForContextBit(b);
					if (curr_level_in_ctx & level)
						return true;
				}
			}
			return false;
		}
	};
}
#endif

