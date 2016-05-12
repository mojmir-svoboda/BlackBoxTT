#pragma once

class Menu;
struct agent;

int     agenttype_mixer_startup ();
int     agenttype_mixer_shutdown ();
int     agenttype_mixer_create (agent *a, char *parameterstring);
int     agenttype_mixer_destroy (agent *a);
int     agenttype_mixer_message (agent *a, int tokencount, char *tokens[]);
void    agenttype_mixer_notify (agent *a, int notifytype, void *messagedata);
void*   agenttype_mixer_getdata (agent *a, int datatype);
void    agenttype_mixer_menu_context (Menu *m, agent *a);
void    agenttype_mixer_notifytype (int notifytype, void *messagedata);


