/*===================================================

	AGENTTYPE_SYSTEMINFO HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_SystemInfo_h
#define BBInterface_AgentType_SystemInfo_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_systeminfo_details
{
	wchar_t *internal_identifier;
	int monitor_type;
};

//Define these functions internally

int agenttype_systeminfo_startup();
int agenttype_systeminfo_shutdown();

int     agenttype_systeminfo_create(agent *a, wchar_t *parameterstring);
int     agenttype_systeminfo_destroy(agent *a);
int     agenttype_systeminfo_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_systeminfo_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_systeminfo_getdata(agent *a, int datatype);
void    agenttype_systeminfo_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_systeminfo_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a);
void    agenttype_systeminfo_notifytype(int notifytype, void *messagedata);

#endif
/*=================================================*/
