#include <iostream>

extern "C"
{
#include <lua.h>
#include <lualib.h>
//#include <lauxlib.h>
}

#include "functions.h"

using namespace std;

#ifndef __BBLUA_H__
#define __BBLUA_H__
class bbLua
{
	public:
		bbLua();
		~bbLua();
		int my_function(lua_State *L);
		void report_errors(lua_State *L, int status);
		void close();
		void init();
		void loadFile(const char* file);
		lua_State* bbLua::GetLuaContext();
		int bbLua::CallLuaFunctionByName(char* function, int arguments, ...);
	private:
		lua_State *LuaContext;
};
#endif
