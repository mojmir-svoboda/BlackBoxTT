//
//	Lua-BB -- Lua plugin for bb4win
//
//	File:		functions.cpp
//	Author:		Noccy
//	Purpose:	Contains exported functions for the LUA interpreter.
//

#include <bbapi.h>
#include <iostream>
#include "functions.h"
#include <winuser.h>
#include <windows.h>

#define BBKONTROLLER_PROXY_CLASSNAME "BBKontroller-Proxy/Class"

/*
	BlackBox API ============================================================
*/

HWND GetBlackboxWindow()
{
	HWND ret=FindWindow("BlackboxClass",NULL);
	if (!ret) ret = FindWindow("xoblite",NULL);
	return(ret);
}

HWND GetProxyWindow()
{
#ifdef __PLUGIN
	HWND ret=0;
#else
	HWND ret=FindWindow(BBKONTROLLER_PROXY_CLASSNAME,NULL);
#endif
	return(ret);
}

/*
	And the respective LUA exports ===========================================
*/

//
//	GetBlackboxWindow (luaGetBlackboxWindow)
//	Retrieves the handle of the blackbox desktop
//
//	In:		Nothing
//	Out:	window handle. Should never be 0.
//
int luaGetBlackboxWindow(lua_State *L)
{
	lua_pushnumber(L, (long)GetBlackboxWindow());
	return(1);
}

//
//	GetProxyWindow (luaGetProxyWindow)
//	Retrieves the handle of the proxy window
//
//	In:		Nothing
//	Out:	window handle. 0 if running as DLL.
//
int luaGetProxyWindow(lua_State *L)
{
	lua_pushnumber(L, (long)GetProxyWindow());
	return(1);
}

//
//	Reconfigure (luaReconfigure)
//	Causes blackbox to reconfigure
//
//	In:		Nothing
//	Out:	Nothing
//
int luaReconfigure(lua_State *L)
{

	// Grab the blackbox hWnd, and cause a reconfiguration
	HWND bbWnd = GetBlackboxWindow();
	SendMessage(bbWnd,BB_RECONFIGURE,0,0);

    return(0);

}

//
//	SwitchDesktop (luaSwitchDesktop)
//	Changes the current desktop to the parameter specified.
//
//	In:		int DesktopToSwitchTo
//	Out:	Nothing
//
int luaSwitchDesktop(lua_State *L)
{
	// Get parameter count
	int argc = lua_gettop(L);
	// If we got enough parameters, go ahead and switch the desktop...
	if (argc == 1) {
		HWND bbWnd = GetBlackboxWindow();
		SendMessage(bbWnd,BB_SWITCHTON,0,(int)lua_tonumber(L,1));
	}
	return(0);
}

//
//	SendBroam (luaSendBroam)
//	Sends a broam
//
//	In:		str BroamToSend
//	Out:	Nothing
//
int luaSendBroam(lua_State *L)
{
	int argc = lua_gettop(L);
	int res = 0;
	if (argc == 1) {
#ifdef __PLUGIN
		HWND bbWnd = GetBlackboxWindow();
		std::cerr << "Sending broam: " << lua_tostring(L,1) << std::endl;
		std::string broam = lua_tostring(L,1);
		res = (SendMessage(bbWnd, BB_BROADCAST, 0, (LPARAM)broam.c_str()));
#else
		HWND bbWnd = GetProxyWindow();
		std::string broam = lua_tostring(L,1);

		if (bbWnd) {
			// Create a COPYDATASTRUCT so we can bass the information on to
			// bbKontroller-Proxy via WM_COPYDATA...
			COPYDATASTRUCT cds;
			memset(&cds,0x00,sizeof(COPYDATASTRUCT));
			cds.dwData = BB_BROADCAST;
			cds.cbData = broam.length() + 1;
			cds.lpData = (LPVOID)broam.c_str();

			// And, relay it to bbKontroller..
			std::cerr << "Sending broam [via proxy]: " << lua_tostring(L,1) << std::endl;
			res = (SendMessage(bbWnd, WM_COPYDATA, 0, (LPARAM)&cds));
		} else {
			std::cerr << "For broams to work from the executable version of lua-bb, the plugin\nbbKontroller-Proxy must be loaded in BlackBox.\n";
		}
#endif
	}
	lua_pushnumber(L, res);
	return(1);
}

int luaPrint(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc == 1) {
		std::string broam = lua_tostring(L,1);
		std::cerr << broam << std::endl;
	}
	return(0);
}

/*
	Timer API ===============================================================
*/






/*
	Events API ==============================================================
*/

//
//	RegisterEvent (luaRegisterEvent)
//	Registers an event
//
//	In:		int MessageType, str FunctionToCall
//	Out:	Index of registered message
//
int luaRegisterEvent(lua_State *L)
{
	// We should have two arguments here
	int argc = lua_gettop(L);
	int ret;
	std::cerr << "Registering event " << lua_tostring(L,2) << std::endl;

	if (argc == 2) {
		// With two arguments we grab a free index in the
		// list (we should also check if the resulting value
		// is -1, in which case too many events have been
		// registered) and grab the values from the LUA stack.
		ret = eventFindFreeIndex();

		int evtype = (int)lua_tonumber(L,1);
		string evfunc = lua_tostring(L,2);

		// Push it into the struct..
		luaEvents[ret].eventType = evtype;
		luaEvents[ret].eventFunc = evfunc;
	} else {
		ret = -1;
	}
	return(ret);
}

int luaUnregisterEvent(lua_State *L)
{
	// Only one argument should be available
	int argc = lua_gettop(L);
	if (argc == 1) {
		// Grab the value
		int idx = (int)lua_tonumber(L,1);

		// Clear the sruct
		luaEvents[idx].eventType = EV_NULL;
		luaEvents[idx].eventFunc = "";
	}
	return(0);
}


/*
	Windows API =============================================================
*/

//
//	FindWindow (luaFindWindow)
//	Finds a window.
//
//	In:		classname, optional windowtitle
//	Out:	window handle. 0 if not found.
//
int luaFindWindow(lua_State *L)
{
	int argc = lua_gettop(L);
	HWND hWndFound = 0;
	std::cerr << "luaFindWindow called with " << argc << " arguments..." << std::endl;
	switch(argc) {
		case 0:
			break;
		case 1:
			std::cerr << "Executing FindWindow('" << lua_tostring(L,1) << "',NULL);" << std::endl;
			hWndFound = FindWindow(lua_tostring(L,1),NULL);
			break;
		case 2:
			std::cerr << "Executing FindWindow('" << lua_tostring(L,1) << "','" << lua_tostring(L,2) << "');" << std::endl;
			hWndFound = FindWindow(lua_tostring(L,1),lua_tostring(L,2));
			break;
		default:
			break;
	}
	std::cerr << "hWnd found is " << hWndFound << std::endl;
	lua_pushnumber(L,(int)hWndFound);
	return(1);
}

//
//	SendMessage (luaSendMessage)
//	Sends a message to a window.
//
//	In:		hwnd, message, lparam, optional wparam
//	Out:	return value of SendMessage call
//
int luaSendMessage(lua_State *L)
{
	int argc = lua_gettop(L);
	// We want at least 3 parameters here... hWnd, Message, and LParam.
	if (argc >= 3) {
		int ret = 0;
		int hWnd = (int)lua_tonumber(L, 1);
		int msg = (int)lua_tonumber(L, 2);
		if (argc == 3) {
			int lp = (int)lua_tonumber(L, 3);
			ret = SendMessage((HWND)hWnd,msg,lp,0);
		} else if (argc == 4) {
			int lp = (int)lua_tonumber(L, 3);
			int wp = (int)lua_tonumber(L, 4);
			ret = SendMessage((HWND)hWnd,msg,lp,wp);
		}
		lua_pushnumber(L,(int)ret);
		return(1);
	}
	return(0);
}


