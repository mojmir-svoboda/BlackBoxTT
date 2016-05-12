//========================================================================
//
//  bbLua.cpp
//
//  The bbLua class. Instantiate this sucker to create a new instance of
//  Lua in the interpreter.
//
//========================================================================

#include "bblua.h"
#include <stdarg.h>

using namespace std;

/*
//	Constructor/destructor. Not used. We stick to init() and close() here.
*/
bbLua::bbLua() { }
bbLua::~bbLua() { }

/*
//  Test function
*/
int bbLua::my_function(lua_State *L)
{
	int argc = lua_gettop(L);

	std::cerr << "-- my_function() called with " << argc
	<< " arguments:" << std::endl;

	for ( int n=1; n<=argc; ++n )
	{
		std::cerr << "-- argument " << n << ": "
		<< lua_tostring(L, n) << std::endl;
	}

	lua_pushnumber(L, 123); // return value
	return 1; // number of return values
}

/*
//  Error handling function
*/
void bbLua::report_errors(lua_State *L, int status)
{
	if ( status!=0 )
	{
		std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1); // remove error message
	}
}

/*
//  bbLua::close - Closes our lua context
*/
void bbLua::close()
{
	lua_close(LuaContext);
}

/*
//	bbLua::init - Initializes our lua context
*/
void bbLua::init()
{
	/* library initialization */
	LuaContext = lua_open();

	luaopen_io(LuaContext); // provides io.*
	luaopen_base(LuaContext);
	luaopen_table(LuaContext);
	luaopen_string(LuaContext);
	luaopen_math(LuaContext);
	luaopen_loadlib(LuaContext);

	// make functions available to Lua programs
	lua_register(LuaContext, "Print", luaPrint);
	lua_register(LuaContext, "Reconfigure", luaReconfigure);
	lua_register(LuaContext, "SwitchDesktop", luaSwitchDesktop);
	lua_register(LuaContext, "SendBroam", luaSendBroam);
	lua_register(LuaContext, "GetBlackboxWindow", luaGetBlackboxWindow);
	lua_register(LuaContext, "GetProxyWindow", luaGetProxyWindow);
	lua_register(LuaContext, "FindWindow", luaFindWindow);
	lua_register(LuaContext, "SendMessage", luaSendMessage);
	lua_register(LuaContext, "RegisterEvent", luaRegisterEvent);
	lua_register(LuaContext, "UnregisterEvent", luaUnregisterEvent);
}

/*
//	loadFile - Loads a file into the lua context.
//
//	This function should be rewritten to handle some kind of #include
//	statement.
*/
void bbLua::loadFile(const char* file) {

	int s = luaL_loadfile(LuaContext, file);

	std::cerr << "Invoking script" << std::endl;

	if ( s==0 )
	{
		s = lua_pcall(LuaContext, 0, LUA_MULTRET, 0);
	}
	report_errors(LuaContext, s);

}


lua_State* bbLua::GetLuaContext()
{
	return(LuaContext);
}

int bbLua::CallLuaFunctionByName(char* function, int arguments, ...)
{
	int i;
	char* val;

	va_list args;
	va_start(args,arguments);

	lua_pushstring(LuaContext,function);
	for (i=0;i<arguments;i++)
	{
		val=va_arg(args,char*);
		lua_pushstring(LuaContext,val);
	}

    lua_pcall(LuaContext,arguments,LUA_MULTRET,0);

	va_end(args);
	return(0);
}
