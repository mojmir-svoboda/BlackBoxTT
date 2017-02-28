/*===================================================

	AGENTTYPE_STATICTEXT HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_StaticText_h
#define BBInterface_AgentType_StaticText_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_statictext_details
{
	wchar_t *text;
};

//Define these functions internally

int agenttype_statictext_startup();
int agenttype_statictext_shutdown();

int     agenttype_statictext_create(agent *a, wchar_t *parameterstring);
int     agenttype_statictext_destroy(agent *a);
int     agenttype_statictext_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_statictext_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_statictext_getdata(agent *a, int datatype);
void    agenttype_statictext_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_statictext_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a);
void    agenttype_statictext_notifytype(int notifytype, void *messagedata);

#endif
/*=================================================*/
