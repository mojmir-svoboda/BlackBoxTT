#include "string.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
//#include "lauxlib.h"
}

const int MAX_EVENTS = 512;

int luaGetBlackboxWindow(lua_State *L);
int luaGetProxyWindow(lua_State *L);
int luaReconfigure(lua_State *L);
int luaGetDesktopInfo(lua_State *L);
int luaShowMenu(lua_State *L);
int luaSwitchDesktop(lua_State *L);
int luaSendBroam(lua_State *L);
int luaPrint(lua_State *L);
int luaFindWindow(lua_State *L);
int luaSendMessage(lua_State *L);
int luaRegisterEvent(lua_State *L);
int luaUnregisterEvent(lua_State *L);

using namespace std;

// Event types
enum {
	EV_NULL,
	EV_RECONFIGURE
};


// The struct we keep our events in.
typedef struct eventData EVENTDATA;
struct eventData 
{
	int 	eventType;
	string	eventFunc;
};
// MAX_EVENTS slots
static EVENTDATA luaEvents[MAX_EVENTS];

inline int eventFindFreeIndex() 
{
	for (int i = 0; i<MAX_EVENTS; i++)
		if (luaEvents[i].eventType == EV_NULL) return(i);
	return(-1);
}
