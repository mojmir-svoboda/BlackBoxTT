#pragma once
#include <bbstring.h>
#include <bbregex.h>

namespace bb {

	struct TaskConfig
	{
		bbstring m_caption;
		bbregex m_caption_regex;
		bool m_ignored;
		bool m_sticky;

		TaskConfig ()
			: m_ignored(false)
			, m_sticky(false)
		{ }

		bool MatchCaption (bbstring const & str)
		{
			return regex_match(str, m_caption_regex);
		}
	};

}

