#pragma once
#include <bbstring.h>
#include <bbregex.h>

namespace bb {

	struct TaskConfig
	{
		bbstring m_caption;
		bbstring m_wspace;
		bbregex m_caption_regex;
		bool m_exclude;
		bool m_ignore;
		bool m_sticky;

		TaskConfig ()
			: m_exclude(false)
			, m_ignore(false)
			, m_sticky(false)
		{ }

		bool MatchCaption (bbstring const & str)
		{
			return regex_match(str, m_caption_regex);
		}
	};

}

