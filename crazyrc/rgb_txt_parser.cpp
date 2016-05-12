#include "rgb_txt_parser.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
//#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/config/warning_disable.hpp>

using namespace boost::spirit::standard_wide;

#include <wtypes.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <tchar.h>
#include <string>
#include <vector>
#include "utils_file.h"

namespace qi = boost::spirit::qi;

#include "color.h"
BOOST_FUSION_ADAPT_STRUCT(Color, (int, r) (int, g) (int, b) (int, a))

namespace rgb_txt {

  rgb_txt_colors g_colorTable;
  rgb_txt_colors const & getColorTable () { return g_colorTable; }
	bool readColorFileToGlobalTable (tstring const & fileName)
  {
    if (g_colorTable.empty())
      return readColorFileFrom(fileName, g_colorTable);
    return false;
  }

	bool reloadColorFileToGlobalTable (tstring const & fileName)
  {
    g_colorTable.clear();
    return readColorFileFrom(fileName, g_colorTable);
  }
}

#include "rgbtxt_dump.h"

namespace rgb_txt {
	template <typename Iterator>
	struct Grammar : qi::grammar<Iterator, file_type(), qi::space_type>
	{
		qi::rule<Iterator, pair_type(), qi::space_type> pair;
		qi::rule<Iterator, color_string_type()> color_name;
		qi::rule<Iterator, Color(), qi::space_type> value_rule;
		qi::rule<Iterator, file_type(), qi::space_type> file;

		Grammar () : Grammar::base_type(file)
		{
			color_name = *(qi::char_ - qi::eol);
			//value_rule %= qi::int_[qi::_a = qi::_1] >> qi::int_[qi::_b = qi::_1] >> qi::int_[qi::_val = boost::phoenix::construct<Color>(qi::_a, qi::_b, qi::_1)];
			value_rule  = qi::int_ >> qi::int_ >> qi::int_ >> qi::attr(0xFF);
			pair = value_rule >> color_name;
			file = * ( pair );
		}
	};

	bool readColorFileFrom (tstring const & fileName, rgb_txt::file_type & result)
	{
		tstring content;
		if (readFileContent(fileName, content))
		{
			static rgb_txt::Grammar<tstring::iterator> const g;
			tstring::iterator begin = content.begin();
			tstring::iterator end = content.end();
			try
			{
				bool const r = qi::phrase_parse(begin, end, g, qi::space, result);

				if (r && begin == end)
				{
					//std::cout << result;
					return true;
				}
				else
				{
					/*std::cout << "+---- parser stopped here\n";
					std::cout << "V\n";
					if (std::distance(begin, end) > 120)
					{
						tstring rest(begin, begin + 120);
						std::cout << rest << "\n";
					}
					else
					{
						tstring rest(begin, end);
						std::cout << rest << "\n";
					}*/
					
					return false;
				}
			}
			catch (...)
			{
				//std::cout << "Parsing failed" << std::endl;
				return false;
			}
			return true;
		}
		return false;
	}
}

