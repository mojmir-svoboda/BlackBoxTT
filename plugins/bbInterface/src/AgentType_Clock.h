/*===================================================

	AGENTTYPE_CLOCK HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_Clock_h
#define BBInterface_AgentType_Clock_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_clock_details
{
	wchar_t *internal_identifier;
	wchar_t *format;
	wchar_t timestr[256];
};

//Define these functions internally

int agenttype_clock_startup();
int agenttype_clock_shutdown();

int     agenttype_clock_create(agent *a, wchar_t *parameterstring);
int     agenttype_clock_destroy(agent *a);
int     agenttype_clock_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_clock_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_clock_getdata(agent *a, int datatype);
void    agenttype_clock_menu_set(Menu *m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_clock_menu_context(Menu *m, agent *a);
void    agenttype_clock_notifytype(int notifytype, void *messagedata);

#endif
/*=================================================*/
