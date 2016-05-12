#pragma once
#include "color.h"
#include "unicode.h"
#include <unordered_map>

namespace rgb_txt {

	typedef tstring color_string_type;
	typedef std::pair<Color, color_string_type> pair_type;
	typedef std::vector<pair_type> file_type;

	//typedef std::unordered_map<tstring, Color> rgb_txt_colors;
	typedef rgb_txt::file_type rgb_txt_colors;
	bool readColorFileFrom (tstring const & fileName, rgb_txt::file_type & result);
	bool readColorFileToGlobalTable (tstring const & fileName);
	bool reloadColorFileToGlobalTable (tstring const & fileName);
	rgb_txt_colors const & getColorTable ();

}

