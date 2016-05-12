#include <windows.h>
#include <cstdio>
#include <tchar.h>
#include <cassert>
#include <string>
#include <vector>

typedef std::basic_string<TCHAR> tstring;
typedef std::vector<tstring> tstrings;

#include "styleitem.h"
#include "rc.h"

int main ()
{
	rc::init();

	tstring result;
	//readString(TEXT("test.rc"), TEXT("blackbox.foo.bar"), TEXT("oops!"), result);
  rc::readString(TEXT("c:\\bb_devel\\test\\bbAnalog.rc"), TEXT("bbanalog.placement"), TEXT("oops!"), result);
	bool result2;
	rc::readBool(TEXT("c:\\bb_devel\\test\\bbslit.rc"), TEXT("bbSlit.alpha.enabled"), false, result2);

	int alpha;
	rc::readInt(TEXT("c:\\bb_devel\\test\\bbslit.rc"), TEXT("bbSlit.alpha.value"), -1, alpha);

	return 0;
}
