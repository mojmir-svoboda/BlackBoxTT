/*===================================================

	AGENTTYPE_DISKSPACEMONITOR HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_DiskSpaceMonitor_h
#define BBInterface_AgentType_DiskSpaceMonitor_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_diskspacemonitor_details
{
	wchar_t *internal_identifier;
	int monitor_type;
	bbstring path;
	wchar_t str_value[16];
	double value;
	double previous_value;
};

//Define these functions internally

int agenttype_diskspacemonitor_startup();
int agenttype_diskspacemonitor_shutdown();

int     agenttype_diskspacemonitor_create(agent *a, wchar_t *parameterstring);
int     agenttype_diskspacemonitor_destroy(agent *a);
int     agenttype_diskspacemonitor_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_diskspacemonitor_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_diskspacemonitor_getdata(agent *a, int datatype);
void    agenttype_diskspacemonitor_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_diskspacemonitor_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a);
void    agenttype_diskspacemonitor_notifytype(int notifytype, void *messagedata);

#endif
/*=================================================*/
