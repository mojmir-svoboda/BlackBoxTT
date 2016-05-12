#include <boost/spirit/include/qi.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/variant/variant.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include "rc_grammar.h"
#include "rc_dump.h"
#include "unicode.h"
#include "utils_file.h"
#include "rc_cache.h"

using namespace boost::spirit::standard_wide;

#include <fstream>
#include <string>
#include <cerrno>
#include <tchar.h>
#include <string>
#include <vector>

#include <windef.h>
#include "rgb_txt_parser.h"

namespace rc {

namespace qi = boost::spirit::qi;

ColorSymbols::ColorSymbols()
{
	for (auto const & item : rgb_txt::getColorTable())
		this->add(item.second, item.first);
}


bool parseFile (tstring const & fname, tstring & err)
{
	tstring content;
	if (readFileContent(fname, content))
	{
		static rc::Grammar<tstring::iterator> const g;

		tstring::iterator begin = content.begin();
		tstring::iterator end = content.end();

		ParsedFileRecord rec;

		try
		{
			bool const r = qi::phrase_parse(begin, end, g, qi::blank, rec.m_parsedFile);

			if (r && begin == end)
			{
				getParsedFileCache().Add(fname, rec);
				getParsedFileCache().MakeIndex();
				//std::cout << m;
				return true;
			}
			else
			{
				tstringstream ss;
				ss << "+---- parser stopped here\n";
				ss << "V\n";
				if (std::distance(begin, end) > 128)
				{
					tstring rest(begin, begin + 128);
					ss << rest << "\n";
				}
				else
				{
					tstring rest(begin, end);
					ss << rest << "\n";
				}
				err = ss.str();
				return false;
			}
		}
		catch (...)
		{
			tstringstream ss;
			ss << "Exception caught while parsing!" << std::endl;
			err = ss.str();
			return false;
		}
		return true;
	}
	return false;
}

bool parseFile (tstring const & fname)
{
	tstring err;
	bool const parsed = parseFile(fname, err);
	if (!parsed)
	{
		//LogError(fname, err);
	}
	return parsed;
}

}

