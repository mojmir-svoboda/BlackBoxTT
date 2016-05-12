#include <blackbox/plugin/bb.h>
#include <string.h>
#include <stdlib.h>
//#ifdef _MSC_VER
//#pragma comment(lib, "winmm.lib")
//#endif
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ControlMaster.h"
#include "ConfigMaster.h"
#include "MenuMaster.h"

#include "AgentType_Mixer.h"
#include "AgentType_Mixer_XP.h"
#include "AgentType_Mixer_Vista.h"

// @TODO: tmp
bool is_vista ()
{
  DWORD const version = GetVersion();
  bool const usingNT = 0 == (version & 0x80000000);
  if (usingNT) {
    DWORD const hex_version = ((version<<8) & 0xFF00) + ((version>>8) & 0x00FF);
    if (bool const usingVista = hex_version >= 0x600)
      return true;
  }
  return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_startup ()
{
	if (is_vista())
		return agenttype_mixer_startup_vista();
	else
		return agenttype_mixer_startup_xp();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_shutdown ()
{
	if (is_vista())
		return agenttype_mixer_shutdown_vista();
	else
		return agenttype_mixer_shutdown_xp();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_create (agent * a, char * parameterstring)
{
	if (is_vista())
		return agenttype_mixer_create_vista(a, parameterstring);
	else
		return agenttype_mixer_create_xp(a, parameterstring);
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_destroy (agent * a)
{
	if (is_vista())
		return agenttype_mixer_destroy_vista(a);
	else
		return agenttype_mixer_destroy_xp(a);
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_message (agent *a, int tokencount, char *tokens[])
{
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixer_notify (agent *a, int notifytype, void *messagedata)
{
	if (is_vista())
		agenttype_mixer_notify_vista(a, notifytype, messagedata);
	else
		agenttype_mixer_notify_xp(a, notifytype, messagedata);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void * agenttype_mixer_getdata (agent *a, int datatype)
{
	if (is_vista())
		return agenttype_mixer_getdata_vista(a, datatype);
	else
		return agenttype_mixer_getdata_xp(a, datatype);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixerscale_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixerscale_menu_set(Menu *m, control *c, agent *a,  char *action, int controlformat)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixerbool_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixerbool_menu_set(Menu *m, control *c, agent *a,  char *action, int controlformat)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixer_menu_context(Menu *m, agent *a)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixer_notifytype (int notifytype, void *messagedata)
{

}

