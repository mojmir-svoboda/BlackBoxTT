#pragma once
#include <tchar.h>
#include <string>
#include <vector>

typedef std::basic_string<TCHAR> tstring;
typedef std::vector<tstring> tstrings;

typedef std::wstringstream tstringstream;
typedef std::wostream tstream;
//HINSTANCE hMainInstance;
/*std::wostream & operator<< (std::wostream & os, std::wstring const & v)
{
  os << v;
	return os;
}*/



