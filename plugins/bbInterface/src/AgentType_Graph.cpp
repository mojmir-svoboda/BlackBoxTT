/*===================================================

	AGENTTYPE_GRAPH CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <string.h>

//Parent Include
#include "AgentType_Graph.h"

//Includes
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "StyleMaster.h"
#include "DialogMaster.h"
#include "MessageMaster.h"
#include "MenuMaster.h"
#include "ColorMaster.h"

//Local variables
const int agenttype_graph_subagentcount = AGENTTYPE_GRAPH_AGENTCOUNT;
#define AGENTTYPE_GRAPH_AGENT_VALUE 0
wchar_t *agenttype_graph_agentdescriptions[AGENTTYPE_GRAPH_AGENTCOUNT] =
{
	L"Value"
};

const int agenttype_graph_agenttypes[AGENTTYPE_GRAPH_AGENTCOUNT] =
{
	CONTROL_FORMAT_SCALE
};

//Timer related stuff
VOID CALLBACK agenttype_graph_timerproc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
int agenttype_graph_counter = 0;
UINT_PTR agenttype_graph_timerid;
list *agenttype_graph_agents;
bool agenttype_graph_hastimer = false;

#define AGENTTYPE_GRAPH_CHARTTYPECOUNT 2
#define AGENTTYPE_GRAPH_CHARTTYPE_FILL 0
#define AGENTTYPE_GRAPH_CHARTTYPE_LINE 1
wchar_t *agenttype_graph_charttypes[AGENTTYPE_GRAPH_CHARTTYPECOUNT] =
{
	L"FILLCHART",
	L"LINECHART"
};

wchar_t *agenttype_graph_friendlycharttypes[AGENTTYPE_GRAPH_CHARTTYPECOUNT] =
{
	L"Filled Chart",
	L"Line Chart"
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//controltype_button_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_graph_startup()
{
	//Create the list
	agenttype_graph_agents = list_create();

	//Register this type with the ControlMaster
	agent_registertype(
		L"Graph",                          //Friendly name of agent type
		L"Graph",                          //Name of agent type
		CONTROL_FORMAT_IMAGE,               //Control type
		true,
		&agenttype_graph_create,          
		&agenttype_graph_destroy,
		&agenttype_graph_message,
		&agenttype_graph_notify,
		&agenttype_graph_getdata,
		&agenttype_graph_menu_set,
		&agenttype_graph_menu_context,
		&agenttype_graph_notifytype
		);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_graph_shutdown()
{
	if(agenttype_graph_hastimer){
		agenttype_graph_hastimer = false;
		KillTimer(NULL, agenttype_graph_timerid);
	}
	//Destroy the internal tracking list
	if (agenttype_graph_agents) list_destroy(agenttype_graph_agents);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_graph_create(agent *a, wchar_t *parameterstring)
{
	//Get the chart type
	int charttype = -1;
	for (int  i = 0; i < AGENTTYPE_GRAPH_CHARTTYPECOUNT; i++)
	{
		if (!_wcsicmp(parameterstring, agenttype_graph_charttypes[i])) charttype = i; 
	}
	if (charttype == -1) return 1;

	//Create the details
	agenttype_graph_details *details = new agenttype_graph_details;
	a->agentdetails = details;
	details->charttype = charttype;
	details->use_custom_color = false;
	details->chartcolor = style_get_text_color(STYLETYPE_TOOLBAR);

	//Create a unique string to assign to this (just a number from a counter)
	wchar_t identifierstring[64];
	swprintf(identifierstring, 64, L"%ul", agenttype_graph_counter);
	details->internal_identifier = new_string(identifierstring);

	//Nullify all agents
	for (int i = 0; i < AGENTTYPE_GRAPH_AGENTCOUNT; i++) details->agents[i] = NULL;

	//Reset the history
	for (int i = 0; i < AGENTTYPE_GRAPH_HISTORYLENGTH; i++) details->valuehistory[i] = 0.0;
	details->historyindex = 0;

	//Increment the counter
	agenttype_graph_counter++;

	//Add this to our internal tracking list
	agent *oldagent; //Unused, but we have to pass it
	list_add(agenttype_graph_agents, details->internal_identifier, (void *) a, (void **) &oldagent);

	//IF this is the first, start the timer
	if (!agenttype_graph_hastimer)
	{
		agenttype_graph_timerid = SetTimer(NULL, 0, 1000, agenttype_graph_timerproc);
		agenttype_graph_hastimer = true;
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_graph_destroy(agent *a)
{
	//Delete the details if possible
	if (a->agentdetails)
	{
		agenttype_graph_details *details =(agenttype_graph_details *) a->agentdetails;

		//Delete all subagents
		for (int i = 0; i < AGENTTYPE_GRAPH_AGENTCOUNT; i++) agent_destroy(&details->agents[i]);		

		//Remove from tracking list
		if (details->internal_identifier != NULL)
		{
			list_remove(agenttype_graph_agents, details->internal_identifier);
			free_string(&details->internal_identifier);
		}

		delete details;
		a->agentdetails = NULL;
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_graph_message(agent *a, int tokencount, wchar_t *tokens[])
{
	agenttype_graph_details *details = (agenttype_graph_details *) a->agentdetails;
	if (!_wcsicmp(L"GraphType", tokens[5]))
	{
		int charttype = -1;
		for (int  i = 0; i < AGENTTYPE_GRAPH_CHARTTYPECOUNT; i++)
		{
			if (!_wcsicmp(tokens[6], agenttype_graph_charttypes[i])) charttype = i; 
		}
		if (charttype == -1) return 1;
		details->charttype = charttype;
		control_notify(a->controlptr, NOTIFY_NEEDUPDATE, NULL);
		return 0;
	}
	else if (!_wcsicmp(L"CustomChartColor", tokens[5]) && config_set_bool(tokens[6],&(details->use_custom_color)))
	{
		control_notify(a->controlptr, NOTIFY_NEEDUPDATE, NULL);
		return 0;
	}
	else if (!_wcsicmp(L"ChartColor",tokens[5])){
		details->use_custom_color = true;
		COLORREF colorval;
		if((colorval = ReadColorFromString(tokens[6])) != -1){
			details->chartcolor = colorval;
			control_notify(a->controlptr, NOTIFY_NEEDUPDATE, NULL);
		}
		return 0;
	}
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_graph_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_graph_details *details = (agenttype_graph_details *) a->agentdetails;
	styledrawinfo *di;
	HPEN hpen;
	HGDIOBJ oldobj;
	int offset_left;
	int width;
	int offset_top;
	int offset_bottom;
	int height;
	int currenty;
	double *currentvalue;
	double finalvalue;
	COLORREF cr;
	wchar_t color[8];

	switch(notifytype)
	{
		case NOTIFY_TIMER:
			
			//Get the current value
			currentvalue = (double *) agent_getdata(details->agents[AGENTTYPE_GRAPH_AGENT_VALUE], DATAFETCH_VALUE_SCALE);
			finalvalue = (currentvalue == NULL || *currentvalue < 0.0 || *currentvalue > 1.0 ? 0.0 : *currentvalue);

			//Store this value in the value history			
			details->historyindex++;
			if (details->historyindex == AGENTTYPE_GRAPH_HISTORYLENGTH) details->historyindex = 0;
			details->valuehistory[details->historyindex] = finalvalue;

			//Notify the control it is time to redraw
			control_notify(a->controlptr, NOTIFY_NEEDUPDATE, NULL);

			break;

		case NOTIFY_DRAW:

			//Get set up for drawing the graph
			di = (styledrawinfo *) messagedata;
			cr = details->use_custom_color ? details->chartcolor : style_get_text_color(STYLETYPE_TOOLBAR);
			hpen = CreatePen(PS_SOLID, 1, cr);
			oldobj = SelectObject(di->buffer, hpen);

			//Prepare some variables for convenience
			offset_left = di->rect.left + 2;
			width = di->rect.right - di->rect.left - 4;
			offset_top = di->rect.top + 2;
			offset_bottom = di->rect.bottom - 2;
			height = di->rect.bottom - di->rect.top - 4;

			//Make sure we have a large enough area
			if (width > 2 && height > 2)
			{
				//Draw the graph
				int currenthistoryindex = details->historyindex;

				if (details->charttype == AGENTTYPE_GRAPH_CHARTTYPE_FILL)
				{
					for (int currentx = width; currentx >= offset_left; currentx--)
					{
						currenty = offset_bottom - (height*details->valuehistory[currenthistoryindex]);
						MoveToEx(di->buffer, currentx, offset_bottom, NULL);
						LineTo(di->buffer, currentx, currenty);
						currenthistoryindex--;
						if (currenthistoryindex < 0) currenthistoryindex = AGENTTYPE_GRAPH_HISTORYLENGTH - 1;
					}
				}
				else if (details->charttype == AGENTTYPE_GRAPH_CHARTTYPE_LINE)
				{
					currenty = offset_bottom - (height*details->valuehistory[currenthistoryindex]);
					for (int currentx = width; currentx >= offset_left; currentx--)
					{						
						MoveToEx(di->buffer, currentx, currenty, NULL);
						currenty = offset_bottom - (height*details->valuehistory[currenthistoryindex]);
						LineTo(di->buffer, currentx-1, currenty);
						currenthistoryindex--;
						if (currenthistoryindex < 0) currenthistoryindex = AGENTTYPE_GRAPH_HISTORYLENGTH - 1;
					}
				}
			}

			//Cleanup
			SelectObject(di->buffer, oldobj);
			DeleteObject(hpen);
		
			break;

		case NOTIFY_SAVE_AGENT:
			//Write existance
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, agenttype_graph_charttypes[details->charttype]));
			if(details->use_custom_color){
				swprintf(color, 8, L"#%06X",switch_rgb(details->chartcolor));
				config_write(config_getfull_control_setagentprop_c(a->controlptr,a->agentaction, L"ChartColor",color));
			}
			//Save all child agents, if necessary
			for (int i = 0; i < AGENTTYPE_GRAPH_AGENTCOUNT; i++) agent_notify(details->agents[i], NOTIFY_SAVE_AGENT, NULL);

			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_graph_getdata(agent *a, int datatype)
{
	agenttype_graph_details *details = (agenttype_graph_details *) a->agentdetails;

	switch (datatype)
	{
		case DATAFETCH_SUBAGENTS_POINTERS_ARRAY:
			return details->agents;
		case DATAFETCH_SUBAGENTS_NAMES_ARRAY:
			return agenttype_graph_agentdescriptions;
		case DATAFETCH_SUBAGENTS_TYPES_ARRAY:
			return (void *) agenttype_graph_agenttypes;
		case DATAFETCH_SUBAGENTS_COUNT:
			return (void *) &agenttype_graph_subagentcount;
	}

	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_graph_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	for (int i = 0; i < AGENTTYPE_GRAPH_CHARTTYPECOUNT; i++)
	{
		make_menuitem_cmd(m, agenttype_graph_friendlycharttypes[i], config_getfull_control_setagent_c(c, action, L"Graph", agenttype_graph_charttypes[i]));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_graph_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
	agenttype_graph_details *details = (agenttype_graph_details *) a->agentdetails;

	for (int i = 0; i < AGENTTYPE_GRAPH_CHARTTYPECOUNT; i++)
	{
		make_menuitem_bol(m, agenttype_graph_friendlycharttypes[i], config_getfull_control_setagentprop_c(a->controlptr, a->agentaction, L"GraphType", agenttype_graph_charttypes[i]), details->charttype == i);
	}
	make_menuitem_nop(m, NULL);

	wchar_t namedot[1000];
	swprintf(namedot, 1000, L"%s%s", a->agentaction, L".");

	menu_controloptions(m, a->controlptr, AGENTTYPE_GRAPH_AGENTCOUNT, details->agents, namedot, agenttype_graph_agentdescriptions, agenttype_graph_agenttypes);
	
	wchar_t color[8];
	swprintf(color, 8, L"#%06X",switch_rgb(details->chartcolor));
	bool temp = !(details->use_custom_color);
	make_menuitem_bol(m, L"Custom Chart Color",config_getfull_control_setagentprop_b(a->controlptr,a->agentaction,L"CustomChartColor",&temp),!temp);
	if(!temp){
		make_menuitem_str(
			m,
			L"Chart Color",
			config_getfull_control_setagentprop_s(a->controlptr,a->agentaction,L"ChartColor"),
			color
		);
	}		
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_graph_notifytype(int notifytype, void *messagedata)
{

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_graph_timerproc
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CALLBACK agenttype_graph_timerproc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	//If we have any agents
	if (agenttype_graph_agents->first != NULL)
	{
		//Notify all agents it is time to update
		listnode *currentnode;
		dolist(currentnode, agenttype_graph_agents)
		{
			agent_notify((agent *) currentnode->value, NOTIFY_TIMER, NULL);
		}
	}
	else
	{
		//If there are no more agents left, kill the timer
		agenttype_graph_hastimer = false;
		KillTimer(NULL, agenttype_graph_timerid);
	}
}

/*=================================================*/
