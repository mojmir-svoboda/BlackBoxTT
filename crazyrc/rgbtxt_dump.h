#pragma once

/*std::ostream & operator<< (std::ostream & os, Color const & v)
{
  os << "rgb:" << v.r << "|" << v.g << "|" << v.b << "|" << v.a;
	return os;
}*/

tstream & operator<< (tstream & os, rgb_txt::pair_type const & p)
{
	os << p.first << ": " << p.second;
	return os;
}

tstream & operator<< (tstream & os, rgb_txt::file_type const & v)
{
	for (rgb_txt::pair_type const & s : v)
		os << s << std::endl;
	return os;
}


