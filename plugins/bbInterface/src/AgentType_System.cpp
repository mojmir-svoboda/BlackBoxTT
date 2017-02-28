/*===================================================

	AGENTTYPE_SYSTEM CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <string.h>

//Parent Include
#include "AgentType_System.h"

//Includes
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "MenuMaster.h"
#include "ConfigMaster.h"

//Define all the appropriate commands
struct agenttype_system_touple
{
	wchar_t *name;
	int msg;
	int wparam;
};

agenttype_system_touple agenttype_system_touples[] = {
	{L"System Shutdown",         BB_SHUTDOWN,      0},
	{L"System Reboot",           BB_SHUTDOWN,      1},
	{L"System Logoff",           BB_SHUTDOWN,      2},
	{L"System Hibernate",        BB_SHUTDOWN,      3},
	{L"System Suspend",          BB_SHUTDOWN,      4},
	{L"System Lock",             BB_SHUTDOWN,      5},
	{L"Show Run Dialog",         BB_RUN,           0},

	{L"Blackbox Quit",           BB_QUIT,          0},
	{L"Blackbox Restart",        BB_RESTART,       0},
	{L"Blackbox Reconfigure",    BB_RECONFIGURE,   0},
	{L"Blackbox Toggle Tray",    BB_TOGGLETRAY,    0},
	{L"Blackbox Toggle Plugins", BB_TOGGLEPLUGINS, 0},

	{L"Blackbox Menu",           BB_MENU, 0},

	{L"Workspace Left",          BB_WORKSPACE,     BBWS_DESKLEFT},
	{L"Workspace Right",         BB_WORKSPACE,     BBWS_DESKRIGHT},
	{L"Workspace Move Left",     BB_WORKSPACE,     BBWS_MOVEWINDOWLEFT  },
	{L"Workspace Move Right",    BB_WORKSPACE,     BBWS_MOVEWINDOWRIGHT },
	{L"Workspace Gather",        BB_WORKSPACE,     BBWS_GATHERWINDOWS},

	{L"Previous Window",         BB_WORKSPACE,     BBWS_PREVWINDOW },
	{L"Next Window",             BB_WORKSPACE,     BBWS_NEXTWINDOW }

};

#define array_count(ary) (sizeof(ary) / sizeof(ary[0]))

const int agenttype_system_touple_count
	= array_count(agenttype_system_touples);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//controltype_button_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_system_startup()
{
	//Register this type with the ControlMaster
	agent_registertype(
		L"System/Shell",                 //Friendly name of agent type
		L"System",                           //Name of agent type
		CONTROL_FORMAT_TRIGGER,             //Control type
		true,
		&agenttype_system_create,           
		&agenttype_system_destroy,
		&agenttype_system_message,
		&agenttype_system_notify,
		&agenttype_system_getdata,
		&agenttype_system_menu_set,
		&agenttype_system_menu_context,
		&agenttype_system_notifytype
		);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_system_shutdown()
{
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_system_create(agent *a, wchar_t *parameterstring)
{
	//Figure out the command
	int index;
	for (index = 0; index < agenttype_system_touple_count; index++)
		if (!_wcsicmp(agenttype_system_touples[index].name, parameterstring))
			goto found;

	return 1;

found:

	//Create the details
	agenttype_system_details *details = new agenttype_system_details;
	a->agentdetails = (void *) details;
	details->commandindex = index;

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_system_destroy(agent *a)
{
	if (a->agentdetails)
	{
		delete (agenttype_system_details *) a->agentdetails;   
		a->agentdetails = NULL;
	}
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_system_message(agent *a, int tokencount, wchar_t *tokens[])
{
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_system_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_system_details *details = (agenttype_system_details *) a->agentdetails;

	switch(notifytype)
	{
		case NOTIFY_CHANGE:
			//Send the message
			PostMessage(
				plugin_hwnd_blackbox,
				agenttype_system_touples[details->commandindex].msg,
				agenttype_system_touples[details->commandindex].wparam,
				0);
			break;

		case NOTIFY_SAVE_AGENT:
			//Write existance
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, agenttype_system_touples[details->commandindex].name));
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_system_getdata(agent *a, int datatype)
{
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_system_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	int set = -1;
	if (a)
	{
		agenttype_system_details *details = (agenttype_system_details *) a->agentdetails;
		set = details->commandindex;
	}

	//List all options
	for (int i = 0; i < agenttype_system_touple_count; i++)
	{
		make_menuitem_bol(m, agenttype_system_touples[i].name, config_getfull_control_setagent_c(c, action, L"System", agenttype_system_touples[i].name), i == set);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_system_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
	make_menuitem_nop(m, L"No options available.");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_system_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_system_notifytype(int notifytype, void *messagedata)
{

}

/*=================================================*/
