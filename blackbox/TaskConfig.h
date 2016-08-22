#pragma once
#include <bbstring.h>
#include <bbregex.h>

namespace bb {

	struct TaskConfig
	{
		bbstring m_caption;
		bbstring m_wspace;
		bbregex m_caption_regex;
		bool m_exclude { false };
		bool m_ignore { false };
		bool m_sticky { false };

		bool MatchCaption (bbstring const & str) const
		{
			return regex_match(str, m_caption_regex);
		}
	};

}

