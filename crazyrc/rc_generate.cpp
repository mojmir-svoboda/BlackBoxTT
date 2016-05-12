#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/variant/variant.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include "rc_grammar.h"
//#include "rc_dump.h"
#include "unicode.h"
#include "utils_file.h"

using namespace boost::spirit::standard_wide;

#include <fstream>
#include <string>
#include <cerrno>
#include <tchar.h>
#include <string>
#include <vector>

#include <windef.h>

typedef std::basic_string<TCHAR> tstring;
typedef std::vector<tstring> tstrings;
//HINSTANCE hMainInstance;

namespace rc {

namespace ka = boost::spirit::karma;

template <typename OutputIterator, typename Container>
bool generate (OutputIterator & sink, Container const& v)
{
    bool r = ka::generate(
        sink,                           // destination: output iterator
        ka::stream % ka::eol,                   // the generator
        v                               // the data to output 
    );
    return r;
}

template <typename OutputIterator>
struct my_grammar : ka::grammar < OutputIterator, file_type()>
{
	my_grammar() : my_grammar::base_type(start)
	{
		start = *(blank | comment | pair);
	}

	ka::rule<OutputIterator, file_type()> start;
	ka::rule<OutputIterator, blank_type()> blank;
	ka::rule<OutputIterator, comment_type()> comment;
	ka::rule<OutputIterator, line_type()> line;
	ka::rule<OutputIterator, pair_type()> pair;
	// more rule declarations...
};

bool writeString (tstring const & fileName, tstring const & szKey, tstring const & defaultString)
{
	typedef std::back_insert_iterator<tstring> iterator_type;

	tstring out;
	iterator_type sink(out);
	my_grammar<iterator_type> g;
	file_type f;
	comment_type c;
	f.push_back(line_type(c));

	bool const result = ka::generate(sink, g, f);
	return result;
}

}
