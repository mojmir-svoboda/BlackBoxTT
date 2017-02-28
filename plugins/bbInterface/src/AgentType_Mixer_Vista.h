#pragma once

class Menu; struct agent; struct control;

int     agenttype_mixer_startup_vista ();
int     agenttype_mixer_shutdown_vista ();
int     agenttype_mixer_create_vista (agent *a, wchar_t *parameterstring);
int     agenttype_mixer_destroy_vista (agent *a);
int     agenttype_mixer_message_vista (agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_mixer_notify_vista (agent *a, int notifytype, void *messagedata);
void*   agenttype_mixer_getdata_vista (agent *a, int datatype);
void    agenttype_mixerscale_menu_set_vista (std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_mixerbool_menu_set_vista (std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_mixer_menu_context_vista (std::shared_ptr<bb::MenuConfig> m, agent *a);
void    agenttype_mixer_notifytype_vista (int notifytype, void *messagedata);

