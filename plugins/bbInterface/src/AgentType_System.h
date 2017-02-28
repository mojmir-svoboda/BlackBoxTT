/*===================================================

	AGENTTYPE_SYSTEM HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_System_h
#define BBInterface_AgentType_System_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_system_details
{
	int commandindex;
};

//Define these functions internally

int agenttype_system_startup();
int agenttype_system_shutdown();

int     agenttype_system_create(agent *a, wchar_t *parameterstring);
int     agenttype_system_destroy(agent *a);
int     agenttype_system_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_system_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_system_getdata(agent *a, int datatype);
void    agenttype_system_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_system_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a);
void    agenttype_system_notifytype(int notifytype, void *messagedata);

#endif
/*=================================================*/
