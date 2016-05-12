#pragma once
#include "unicode.h"
#include <fstream>

#include <codecvt>
/*std::string source;


std::u16string dest = convert.from_bytes(source);

std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;

UTF - 8 to UTF - 16

std::string source;
...
std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
std::u16string dest = convert.from_bytes(source);

UTF - 16 to UTF - 8

std::u16string source;
...
std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
std::string dest = convert.to_bytes(source);*/

inline bool readFileContent (tstring const & filename, tstring & tresult)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in)
	{
		tstringstream str;
		std::wifstream file(filename);
		file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));

		tstring line;
		while (std::getline(file, line))
		{
			str << line << std::endl;
			//... do something processing the UTF - 16 string ...
		}
		tresult = str.str();
		//file >> tresult;
		//for (wchar_t c; file >> c;)
		//	tresult.append(c);
		//tresult = str.str();
		//std::cout << "UTF-16 read from the same file (using codecvt_utf8_utf16)\n";
		//for (wchar_t c; file2 >> c;)
		//	std::cout << std::hex << std::showbase << c << '\n';

    /*std::string result;
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&result[0], result.size());

    //tresult = UTF8ToUTF16(result);

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    tresult = convert.from_bytes(result);
    //std::u16string dest = convert.from_bytes(result);


		in.close();*/
		return true;
	}
	return false;
}


