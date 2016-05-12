#pragma once
#include <mmsystem.h>
#include "AgentMaster.h"
#include "ControlMaster.h"

int     agenttype_mixer_startup_xp ();
int     agenttype_mixer_shutdown_xp ();
int     agenttype_mixer_create_xp (agent *a, char *parameterstring);
int     agenttype_mixer_destroy_xp (agent *a);
int     agenttype_mixer_message_xp (agent *a, int tokencount, char *tokens[]);
void    agenttype_mixer_notify_xp (agent *a, int notifytype, void *messagedata);
void*   agenttype_mixer_getdata_xp (agent *a, int datatype);
void    agenttype_mixerscale_menu_set_xp (Menu *m, control *c, agent *a,  char *action, int controlformat);
void    agenttype_mixerbool_menu_set_xp (Menu *m, control *c, agent *a,  char *action, int controlformat);
void    agenttype_mixer_menu_context_xp (Menu *m, agent *a);
void    agenttype_mixer_notifytype_xp (int notifytype, void *messagedata);


