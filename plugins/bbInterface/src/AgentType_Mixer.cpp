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
	Assert(is_vista());
	return agenttype_mixer_startup_vista();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_shutdown ()
{
	return agenttype_mixer_shutdown_vista();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_create (agent * a, wchar_t * parameterstring)
{
	Assert(is_vista());
	return agenttype_mixer_create_vista(a, parameterstring);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_destroy (agent * a)
{
	return agenttype_mixer_destroy_vista(a);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_mixer_message (agent *a, int tokencount, wchar_t *tokens[])
{
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixer_notify (agent *a, int notifytype, void *messagedata)
{
	agenttype_mixer_notify_vista(a, notifytype, messagedata);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void * agenttype_mixer_getdata (agent *a, int datatype)
{
	return agenttype_mixer_getdata_vista(a, datatype);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixerscale_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixerscale_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  char *action, int controlformat)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixerbool_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixerbool_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  char *action, int controlformat)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixer_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_mixer_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_mixer_notifytype (int notifytype, void *messagedata)
{

}

