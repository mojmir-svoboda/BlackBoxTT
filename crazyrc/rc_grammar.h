#pragma once
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/home/qi/nonterminal/error_handler.hpp>
#include <boost/config/warning_disable.hpp>
//#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <string>
#include <cstdlib>

#include <iostream>

using namespace boost::spirit::standard_wide;

#include "color.h"
#include "rc_types.h"

// http://boost-spirit.com/home/articles/qi-example/zero-to-60-mph-in-2-seconds/:

/* rc format:
http://www.tldp.org/HOWTO/XWindow-User-HOWTO/moreconfig.html
http://www.linuxdocs.org/HOWTOs/XWindow-User-HOWTO-8.html
http://www.designmatrix.com/services/XResources_syntax.html
*/

namespace qi = boost::spirit::qi;

BOOST_FUSION_ADAPT_STRUCT(rc::EmptyLine, (tstring, spaces))
BOOST_FUSION_ADAPT_STRUCT(Color, (int, r) (int, g) (int, b) (int, a))

namespace rc {
	struct ColorSymbols : qi::symbols<char, Color > { ColorSymbols(); };

	template <typename Iterator>
	struct Grammar : qi::grammar<Iterator, file_type(), qi::blank_type>
	{
		qi::rule<Iterator, comment_type()> c_comment;
		qi::rule<Iterator, comment_type()> cpp_comment;
		qi::rule<Iterator, comment_type()> rc_comment;
		qi::rule<Iterator, comment_type()> comment;
		qi::rule<Iterator, pair_type(), qi::blank_type> pair;
		qi::rule<Iterator, key_type()> key;
		qi::rule<Iterator, value_string_type()> value_string;
		qi::rule<Iterator, value_bool_type()> value_bool;
		qi::rule<Iterator, value_int_type()> value_int;
		qi::rule<Iterator, value_type()> value;
		qi::rule<Iterator, value_color_type()> value_colorref_hex;
		qi::rule<Iterator, value_color_type()> value_colorref_rgb;
		qi::rule<Iterator, value_color_type()> value_colorref_rgba;
		qi::rule<Iterator, value_color_type()> value_color;
		qi::rule<Iterator, EmptyLine(), qi::blank_type> blank;
		qi::rule<Iterator, file_type(), qi::blank_type> file;
		ColorSymbols color_name;

		Grammar () : Grammar::base_type(file)
		{
			// comments
			c_comment   %= boost::spirit::repository::confix("/*", "*/")[ *(char_ - "*/") ]; // -> str
			cpp_comment %= boost::spirit::repository::confix('#', qi::eol | qi::eoi)[ *(char_ - qi::eol) ]; // -> str
			rc_comment %= boost::spirit::repository::confix('!', qi::eol | qi::eoi)[ *(char_ - qi::eol) ]; // -> str
			comment     =  c_comment | cpp_comment | rc_comment; // variant<str, str> -> str

			key = qi::char_("a-zA-Z_*") >> *qi::char_("a-zA-Z_0-9./*");
			value_bool = qi::true_ | qi::false_;
			value_int = qi::int_;
			value_string = *(qi::char_ - qi::eol);
			// color
			value_colorref_hex = ('#' >> qi::hex);
			value_colorref_rgb  = qi::lit("rgb:")  >> qi::hex >> '/' >> qi::hex >> '/' >> qi::hex >> qi::attr(0xFF);
			value_colorref_rgba = qi::lit("rgba:") >> qi::hex >> '/' >> qi::hex >> '/' >> qi::hex >> '/' >> qi::hex;
			value_color = value_colorref_hex | value_colorref_rgb | color_name;
			//value_color = value_colorref_hex | value_colorref_rgb | value_colorref_sym;

			value = value_color | value_bool | value_int | value_string; //  -> variant<str, color, int, bool>
			pair = key >> ':' >> -qi::lexeme[ value ] >> (qi::eol | qi::eoi);

			blank = qi::eol[qi::_val = boost::phoenix::construct<EmptyLine>()];
			file = *(blank | comment | pair);

			/*using boost::phoenix::val;
			boost::spirit::qi::on_error<boost::spirit::qi::fail>
				(
				file
				, std::cout
				<< val("Error! Expecting ")
				<< boost::spirit::qi::_4                               // what failed?
				<< val(" here: \"")
				<< boost::phoenix::construct<tstring>(boost::spirit::qi::_3, boost::spirit::qi::_2)   // iterators to error-pos, end
				<< val("\"")
				<< std::endl
				);*/
		}
	};
}

