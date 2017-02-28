/*===================================================

	AGENTTYPE_RUN CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <string.h>

//Parent Include
#include "AgentType_Run.h"

//Includes
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "DialogMaster.h"
#include "MessageMaster.h"
#include "MenuMaster.h"

//Constants

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//controltype_button_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_run_startup()
{
	//Register this type with the ControlMaster
	agent_registertype(
		L"Run/Open",                 //Friendly name of agent type
		L"Run",                      //Name of agent type
		CONTROL_FORMAT_TRIGGER|CONTROL_FORMAT_DROP, //Control type
		true,
		&agenttype_run_create,          
		&agenttype_run_destroy,
		&agenttype_run_message,
		&agenttype_run_notify,
		&agenttype_run_getdata,
		&agenttype_run_menu_set,
		&agenttype_run_menu_context,
		&agenttype_run_notifytype
		);


	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_run_shutdown()
{
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_run_create(agent *a, wchar_t *parameterstring)
{
	if (0 == * parameterstring)
		return 2; // no param, no agent

	//Create the details
	agenttype_run_details *details = new agenttype_run_details;
	a->agentdetails = (void *) details;
	details->command = NULL;
	details->arguments = NULL;
	details->workingdir = NULL;

	//Copy the parameter string
	if (!_wcsicmp(parameterstring, L"*browse*"))
	{
		//Get the file
		wchar_t *file = dialog_file(szFilterAll, L"Select File", NULL, NULL, false);
		if (file)
		{
			details->command = new_string(file);
		}
		else
		{
			//message_override = true;
			return 2;
		}
	}
	else
	{
		details->command = new_string(parameterstring);
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_run_destroy(agent *a)
{
	if (a->agentdetails)
	{
		agenttype_run_details *details = (agenttype_run_details *) a->agentdetails;
		free_string(&details->command);
		free_string(&details->arguments);
		free_string(&details->workingdir);
		delete details;
		a->agentdetails = NULL;
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_run_message(agent *a, int tokencount, wchar_t *tokens[])
{
	if (!_wcsicmp(L"Arguments", tokens[5]))
	{
		agenttype_run_details *details = (agenttype_run_details *) a->agentdetails;
		free_string(&details->arguments);
		if (*tokens[6]) details->arguments = new_string(tokens[6]);
		return 0;
	}
	if (!_wcsicmp(L"WorkingDir", tokens[5]))
	{
		agenttype_run_details *details = (agenttype_run_details *) a->agentdetails;
		free_string(&details->workingdir);
		if (*tokens[6]) details->workingdir = new_string(tokens[6]);
		return 0;
	}
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_run_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_run_details *details;
	details = (agenttype_run_details *) a->agentdetails;

	switch(notifytype)
	{
		case NOTIFY_CHANGE:
		{
			const wchar_t *arguments = details->arguments;
			if (a->format & CONTROL_FORMAT_DROP) // OnDrop
			{
				if (NULL == arguments) arguments = L"$DroppedFile$";
			}
			wchar_t buffer[BBI_MAX_LINE_LENGTH];
			if (arguments)
				arguments = message_preprocess(wcscpy(buffer, arguments), a->controlptr->moduleptr);
			shell_exec(details->command, arguments, details->workingdir);
			break;
		}
		case NOTIFY_SAVE_AGENT:
			//Write existance
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, details->command));
			//Write arguments
			if (details->arguments) config_write(config_get_control_setagentprop_c(a->controlptr, a->agentaction, L"Arguments", details->arguments));
			if (details->workingdir) config_write(config_get_control_setagentprop_c(a->controlptr, a->agentaction, L"WorkingDir", details->workingdir));
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_run_getdata(agent *a, int datatype)
{
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_run_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	make_menuitem_str(m, L"Entry:", config_getfull_control_setagent_s(c, action, L"Run"),
		a ? ((agenttype_run_details *) a->agentdetails)->command : L"");
	make_menuitem_cmd(m, L"Browse...", config_getfull_control_setagent_c(c, action, L"Run", L"*browse*"));

	make_menuitem_nop(m, L"");
	make_menuitem_cmd(m, L"Notepad", config_getfull_control_setagent_c(c, action, L"Run", L"notepad.exe"));
	make_menuitem_cmd(m, L"Calculator", config_getfull_control_setagent_c(c, action, L"Run", L"calc.exe"));
	make_menuitem_cmd(m, L"Command Prompt", config_getfull_control_setagent_c(c, action, L"Run", L"cmd.exe"));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_run_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
	//Get the agent details
	agenttype_run_details *details = (agenttype_run_details *) a->agentdetails;

	//Arguments
	make_menuitem_str(m, L"Arguments", config_getfull_control_setagentprop_s(a->controlptr, a->agentaction, L"Arguments"),
		details->arguments ? details->arguments : L"");

	make_menuitem_str(m, L"Working Dir", config_getfull_control_setagentprop_s(a->controlptr, a->agentaction, L"WorkingDir"),
		details->workingdir ? details->workingdir : L"");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_run_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_run_notifytype(int notifytype, void *messagedata)
{

}


/*=================================================*/
