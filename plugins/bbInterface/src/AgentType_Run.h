/*===================================================

	AGENTTYPE_RUN HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_Run_h
#define BBInterface_AgentType_Run_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_run_details
{
	wchar_t *command;
	wchar_t *arguments;
	wchar_t *workingdir;
};

//Define these functions internally

int agenttype_run_startup();
int agenttype_run_shutdown();

int     agenttype_run_create(agent *a, wchar_t *parameterstring);
int     agenttype_run_destroy(agent *a);
int     agenttype_run_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_run_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_run_getdata(agent *a, int datatype);
void    agenttype_run_menu_set(Menu *m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_run_menu_context(Menu *m, agent *a);
void    agenttype_run_notifytype(int notifytype, void *messagedata);

#endif
/*=================================================*/
