/*===================================================

	AGENTTYPE_GRAPH HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_AutoScaleGraph_h
#define BBInterface_AgentType_AutoScaleGraph_h

//Includes
#include "AgentMaster.h"

//Define these structures
#define AGENTTYPE_AUTOSCALEGRAPH_AGENTCOUNT 1
#define AGENTTYPE_AUTOSCALEGRAPH_HISTORYLENGTH 1000
struct agenttype_autoscalegraph_details
{
	wchar_t *internal_identifier;
	agent *agents[1];
	double valuehistory[AGENTTYPE_AUTOSCALEGRAPH_HISTORYLENGTH];
	int historyindex;
	int charttype;
	double range;
	int rangecount;
	bool use_custom_color;
	COLORREF chartcolor;
	bool use_user_range;
	double user_range;
};

//Define these functions internally

int agenttype_autoscalegraph_startup();
int agenttype_autoscalegraph_shutdown();

int     agenttype_autoscalegraph_create(agent *a, wchar_t *parameterstring);
int     agenttype_autoscalegraph_destroy(agent *a);
int     agenttype_autoscalegraph_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_autoscalegraph_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_autoscalegraph_getdata(agent *a, int datatype);
void    agenttype_autoscalegraph_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a, wchar_t *action, int controlformat);
void    agenttype_autoscalegraph_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a);
void    agenttype_autoscalegraph_notifytype(int notifytype, void *messagedata);

void    agenttype_icon_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat);
int     agenttype_icon_create(agent *a, wchar_t *parameterstring);

#endif
/*=================================================*/
