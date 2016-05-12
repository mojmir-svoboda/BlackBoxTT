#pragma once

/*std::ostream & operator<< (std::ostream & os, rc::value_color_type const & v)
{
  os << "rgb:" << v.r << "|" << v.g << "|" << v.b << "|" << v.a;
	return os;
}*/

tstream & operator<< (tstream & os, rc::value_variant_type const & v)
{
	switch (v.which())
	{
    case rc::e_val_bool:
    {
      bool const b = boost::get<rc::value_bool_type>(v);
      os << (b ? "true" : "false");
      return os;
    }
    case rc::e_val_int: 
    {
      int const i = boost::get<rc::value_int_type>(v);
      os << i;
      return os;
    }
    case rc::e_val_string: 
    {
      tstring const & s = boost::get<rc::value_string_type>(v);
      os << s;
      return os;
    }
    case rc::e_val_color: 
    {
      Color const & c = boost::get<rc::value_color_type>(v);
      os << c;
      return os;
    }
	}
	return os;
}

tstream & operator<< (tstream & os, rc::value_type const & v)
{
	if (v)
		os << *v;
	else
		os << "N/A";
	return os;
}

tstream & operator<< (tstream & os, rc::EmptyLine const & v)
{
  //os << "CRLF" << std::endl;
  os << std::endl;
	return os;
}


tstream & operator<< (tstream & os, rc::pair_type const & p)
{
	os << p.first << ": " << p.second;
	return os;
}

tstream & operator<< (tstream & os, rc::line_type const & l)
{
	switch (l.which())
  {
	case rc::e_line_comment: os << boost::get<rc::comment_type>(l); return os;
	case rc::e_line_pair: os << boost::get<rc::pair_type>(l); return os;
	case rc::e_line_blank: os << boost::get<rc::blank_type>(l); return os;
  }
	return os;
}

tstream & operator<< (tstream & os, rc::file_type const & v)
{
	for (rc::line_type const & s : v)
		os << s << std::endl;
	return os;
}


