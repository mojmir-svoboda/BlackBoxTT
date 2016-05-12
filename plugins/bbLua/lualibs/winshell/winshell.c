#include <windows.h>
#include "lauxlib.h"

/* Pop-up a Windows message box with your choice of message and caption */
int lua_msgbox(lua_State* L)
{
	const char* message = luaL_checkstring(L, 1);
	const char* caption = luaL_optstring(L, 2, "");
	int result = MessageBox(NULL, message, caption, MB_OK);
	lua_pushnumber(L, result);
	return 1;
}

int __declspec(dllexport) libinit (lua_State* L)
{
	lua_register(L, "msgbox",  lua_msgbox);
	return 0;
}
