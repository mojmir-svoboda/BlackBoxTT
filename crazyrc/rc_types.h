#pragma once
#include <boost/assert.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/optional/optional.hpp>
#include "color.h"
#include "unicode.h"
#include <vector>

//namespace qi = boost::spirit::qi;

namespace rc {
	struct EmptyLine
	{
		tstring spaces;
	};
}

namespace rc {

	typedef tstring key_type;
	typedef bool value_bool_type;
	typedef int value_int_type;
	typedef COLORREF value_colorref_type;
	typedef Color value_color_type;
	typedef tstring value_string_type;
	typedef EmptyLine blank_type;

	enum { e_val_string, e_val_color, e_val_int, e_val_bool };
	typedef boost::variant<value_string_type, value_color_type, value_int_type, value_bool_type> value_variant_type;
	typedef boost::optional<value_variant_type> value_type;
	typedef tstring comment_type;
	typedef std::pair<key_type, value_type> pair_type;
	enum { e_line_comment, e_line_pair, e_line_blank };
	typedef boost::variant<comment_type, pair_type, blank_type> line_type;
	typedef std::vector<line_type> file_type;
	typedef file_type rc_file;

}

