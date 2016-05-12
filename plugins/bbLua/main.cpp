#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <windows.h>
#include "bblua.h"

int main(int argc, char** argv)
{
	bbLua bbLuaInstance;

	bbLuaInstance.init();
	bbLuaInstance.loadFile(argv[1]);

	std::cerr << "Entering interactive mode...\n" \
		"  exit  -- Will exit this session, tyvm :)\n" \
		"  help  -- Will give you some help.\n";

	std::string cmd;
	bool cont = true;
	while (cont)
	{
		std::cerr << "[bbLua] ";
		std::cin >> cmd;
		if (cmd == "exit") {
			cont = false;
		}
	}


	std::cerr << "Closing bbLua..." << std::endl;

	bbLuaInstance.close();
    return 0;
}
