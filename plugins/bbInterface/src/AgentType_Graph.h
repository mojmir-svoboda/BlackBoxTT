/*===================================================

	AGENTTYPE_GRAPH HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_Graph_h
#define BBInterface_AgentType_Graph_h

//Includes
#include "AgentMaster.h"

//Define these structures
#define AGENTTYPE_GRAPH_AGENTCOUNT 1
#define AGENTTYPE_GRAPH_HISTORYLENGTH 1000
struct agenttype_graph_details
{
	wchar_t *internal_identifier;
	agent *agents[1];
	double valuehistory[AGENTTYPE_GRAPH_HISTORYLENGTH];
	int historyindex;
	int charttype;
	bool use_custom_color;
	COLORREF chartcolor;
};

//Define these functions internally

int agenttype_graph_startup();
int agenttype_graph_shutdown();

int     agenttype_graph_create(agent *a, wchar_t *parameterstring);
int     agenttype_graph_destroy(agent *a);
int     agenttype_graph_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_graph_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_graph_getdata(agent *a, int datatype);
void    agenttype_graph_menu_set(Menu *m, control *c, agent *a, wchar_t *action, int controlformat);
void    agenttype_graph_menu_context(Menu *m, agent *a);
void    agenttype_graph_notifytype(int notifytype, void *messagedata);

void    agenttype_icon_menu_set(Menu *m, control *c, agent *a,  wchar_t *action, int controlformat);
int     agenttype_icon_create(agent *a, wchar_t *parameterstring);

#endif
/*=================================================*/
