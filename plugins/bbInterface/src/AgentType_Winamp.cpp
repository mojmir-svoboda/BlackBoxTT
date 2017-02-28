/*===================================================

	AGENTTYPE_WINAMP CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>

//Parent Include
#include "AgentType_Winamp.h"

//Includes
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "MenuMaster.h"
#include "ListMaster.h"
#include "WindowMaster.h"
#include "ControlMaster.h"

#define array_count(ary) (sizeof(ary) / sizeof(ary[0]))

//Local variables
HWND winamp_hwnd = NULL;
const wchar_t agenttype_winamp_timerclass[] = L"BBInterfaceAgentWinamp";
bool agenttype_winamp_windowclassregistered;
LRESULT CALLBACK agenttype_winamp_event(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK agenttype_winamp_timercall(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
HWND agenttype_winamp_window;
unsigned long agenttype_winamp_counter;
list *agenttype_winamp_agents;

//The pieces of data that might be fetched
bool agenttype_winamp_isplaying = false;
double agenttype_winamp_trackposition = 0.0;
wchar_t agenttype_winamp_titlebuffer[1024] = L"";
wchar_t agenttype_winamp_timeelapsed[32] = L"";
wchar_t agenttype_winamp_timeremaining[32] = L"";
wchar_t agenttype_winamp_timetotal[32] = L"";
wchar_t *agenttype_winamp_currenttitle = agenttype_winamp_titlebuffer;
wchar_t agenttype_winamp_bitrate[32] = L"";

//Function prototypes
bool agenttype_winamp_getwinampwindow();
void agenttype_winamppoller_updatevalues();

//Definitions
#define WINAMP_POLLINGTYPECOUNT 8

#define WINAMP_POLLINGTYPE_NONE 0
#define WINAMP_POLLINGTYPE_ISPLAYING 1
#define WINAMP_POLLINGTYPE_POSITION 2
#define WINAMP_POLLINGTYPE_LASTSCALE 2
#define WINAMP_POLLINGTYPE_TITLE 3
#define WINAMP_POLLINGTYPE_TIME_ELAPSED 4
#define WINAMP_POLLINGTYPE_TIME_REMAINING 5
#define WINAMP_POLLINGTYPE_TIME_TOTAL 6
#define WINAMP_POLLINGTYPE_BITRATE 7

const wchar_t *agenttype_winamp_pollingnames[WINAMP_POLLINGTYPECOUNT] =
{
	L"None",
	L"IsPlaying",
	L"TrackPosition",
	L"TrackTitle",
	L"TimeElapsed",
	L"TimeRemaining",
	L"TimeTotal",
	L"Bitrate"
};

const wchar_t *agenttype_winamp_friendlynames[WINAMP_POLLINGTYPECOUNT] =
{
	L"None",
	L"Track Is Playing",
	L"Track Position",
	L"Track Title",
	L"Time Elapsed",
	L"Time Remaining",
	L"Time Total",
	L"Bitrate"
};

//Definitions
struct { int code; const wchar_t *string; } winamp_actions[] =
{
	{ 40044, L"Previous track button"                          },
	{ 40048, L"Next track button"                              },
	{ 40045, L"Play button"                                    },
	{ 40046, L"Pause/Unpause button"                           },
	{ 40047, L"Stop button"                                    },
	{ 40147, L"Fadeout and stop"                               },
	{ 40157, L"Stop after current track"                       },
	{ 40148, L"Fast-forwardseconds"                            },
	{ 40144, L"Fast-rewindseconds"                             },
	{ 40154, L"Start of playlist"                              },
	{ 40158, L"Go to end of playlist "                         },
	{ 40029, L"Open file dialog"                               },
	{ 40155, L"Open URL dialog"                                },
	{ 40188, L"Open file info box"                             },
	{ 40037, L"Set time display mode to elapsed"               },
	{ 40038, L"Set time display mode to remaining"             },
	{ 40012, L"Toggle preferences screen"                      },
	{ 40190, L"Open visualization options"                     },
	{ 40191, L"Open visualization plug-in options"             },
	{ 40192, L"Execute current visualization plug-in"          },
	{ 40041, L"Toggle about box"                               },
	{ 40189, L"Toggle title Autoscrolling"                     },
	{ 40019, L"Toggle always on top"                           },
	{ 40064, L"Toggle Windowshade"                             },
	{ 40266, L"Toggle Playlist Windowshade"                    },
	{ 40165, L"Toggle doublesize mode"                         },
	{ 40036, L"Toggle EQ"                                      },
	{ 40040, L"Toggle playlist editor"                         },
	{ 40258, L"Toggle main window visible"                     },
	{ 40298, L"Toggle minibrowser"                             },
	{ 40186, L"Toggle easymove"                                },
	{ 40058, L"Raise volume by 1%"                             },
	{ 40059, L"Lower volume by 1%"                             },
	{ 40022, L"Toggle repeat"                                  },
	{ 40023, L"Toggle shuffle"                                 },
	{ 40193, L"Open jump to time dialog"                       },
	{ 40194, L"Open jump to file dialog"                       },
	{ 40219, L"Open skin selector"                             },
	{ 40221, L"Configure current visualization plug-in"        },
	{ 40291, L"Reload the current skin"                        },
	{ 40001, L"Close Winamp"                                   },
	{ 40197, L"Moves backtracks in playlist"                   },
	{ 40320, L"Show the edit bookmarks"                        },
	{ 40321, L"Adds current track as a bookmark"               },
	{ 40323, L"Play audio CD"                                  },
	{ 40253, L"Load a preset from EQ"                          },
	{ 40254, L"Save a preset to EQF"                           },
	{ 40172, L"Opens load presets dialog"                      },
	{ 40173, L"Opens auto-load presets dialog"                 },
	{ 40174, L"Load default preset"                            },
	{ 40175, L"Opens save preset dialog"                       },
	{ 40176, L"Opens auto-load save preset"                    },
	{ 40178, L"Opens delete preset dialog"                     },
	{ 40180, L"Opens delete an auto load preset dialog"        },
	{ 40379, L"Show media library"                             },
	{ 40380, L"Hide media library"                             }
};

const int winamp_actions_count = array_count(winamp_actions);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_winamp_startup()
{
	//Register the window class
	agenttype_winamp_windowclassregistered = false;
	if (window_helper_register(agenttype_winamp_timerclass, &agenttype_winamp_event))
	{
		//Couldn't register the window
		return 1;
	}
	agenttype_winamp_windowclassregistered = true;

	//Create the window
	agenttype_winamp_window = window_helper_create(agenttype_winamp_timerclass);
	if (!agenttype_winamp_window)
	{
		//Couldn't create the window
		return 1;
	}

	//Create the list
	agenttype_winamp_agents = list_create();

	//Register this type with the ControlMaster
	agent_registertype(
		L"Winamp",                           //Friendly name of agent type
		L"Winamp",                           //Name of agent type
		CONTROL_FORMAT_TRIGGER,             //Control format
		true,
		&agenttype_winamp_create,           
		&agenttype_winamp_destroy,
		&agenttype_winamp_message,
		&agenttype_winamp_notify,
		&agenttype_winamp_getdata,
		&agenttype_winamp_menu_set,
		&agenttype_winamp_menu_context,
		&agenttype_winamp_notifytype
		);

	//Register this type with the ControlMaster
	agent_registertype(
		L"Winamp",                           //Friendly name of agent type
		L"WinampScale",                      //Name of agent type
		CONTROL_FORMAT_SCALE,               //Control format
		true,
		&agenttype_winamppoller_create,           
		&agenttype_winamp_destroy,
		&agenttype_winamp_message,
		&agenttype_winamppoller_notify,
		&agenttype_winamppoller_getdata,
		&agenttype_winamppoller_scale_menu_set,
		&agenttype_winamp_menu_context,
		&agenttype_winamp_notifytype
		);

	//Register this type with the ControlMaster
	agent_registertype(
		L"Winamp",                           //Friendly name of agent type
		L"WinampBool",                       //Name of agent type
		CONTROL_FORMAT_BOOL,                //Control format
		false,
		&agenttype_winamppoller_create,           
		&agenttype_winamp_destroy,
		&agenttype_winamp_message,
		&agenttype_winamppoller_notify,
		&agenttype_winamppoller_getdata,
		&agenttype_winamppoller_bool_menu_set,
		&agenttype_winamp_menu_context,
		&agenttype_winamp_notifytype
		);

	//Register this type with the ControlMaster
	agent_registertype(
		L"Winamp",                           //Friendly name of agent type
		L"WinampText",                       //Name of agent type
		CONTROL_FORMAT_TEXT,                //Control format
		true,
		&agenttype_winamppoller_create,           
		&agenttype_winamp_destroy,
		&agenttype_winamp_message,
		&agenttype_winamppoller_notify,
		&agenttype_winamppoller_getdata,
		&agenttype_winamppoller_text_menu_set,
		&agenttype_winamp_menu_context,
		&agenttype_winamp_notifytype
		);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_winamp_shutdown()
{
	//Destroy the internal tracking list
	if (agenttype_winamp_agents) list_destroy(agenttype_winamp_agents);

	//Destroy the window
	if (agenttype_winamp_window) window_helper_destroy(agenttype_winamp_window);

	//Unregister the window class
	if (agenttype_winamp_windowclassregistered) window_helper_unregister(agenttype_winamp_timerclass);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_winamp_create(agent *a, wchar_t *parameterstring)
{
	int code = _wtoi(parameterstring);
	if (code < 40001 || code > 40394) return 1;

	//Create the details
	agenttype_winamp_details *details = new agenttype_winamp_details;
	a->agentdetails = (void *) details;

	//Copy the code
	details->commandcode = code;
	details->internal_identifier = NULL;

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_winamppoller_create(agent *a, wchar_t *parameterstring)
{
	//Figure out what type of agent this is
	int commandcode = 0;	
	if (!_wcsicmp(a->agenttypeptr->agenttypename, L"WinampScale") && !_wcsicmp(parameterstring, L"TrackPosition")) commandcode = WINAMP_POLLINGTYPE_POSITION;
	else if (!_wcsicmp(a->agenttypeptr->agenttypename, L"WinampBool") && !_wcsicmp(parameterstring, L"IsPlaying")) commandcode = WINAMP_POLLINGTYPE_ISPLAYING;
	else
	{
		for (int i = WINAMP_POLLINGTYPE_LASTSCALE + 1; i < WINAMP_POLLINGTYPECOUNT; i++)
		{
			if (!_wcsicmp(a->agenttypeptr->agenttypename, L"WinampText") && !_wcsicmp(parameterstring, agenttype_winamp_pollingnames[i])) commandcode = i;
		}
	}
	if (commandcode == 0) return 1;


	//If it is an invalid agent code, say so
	if (commandcode == 0)
	{
		wchar_t tempstring[1000];
		swprintf(tempstring, 1000, L"There was an error setting the WinAmp agent:\n\nThe type %s %s is not a valid type.", a->agenttypeptr->agenttypename, parameterstring);
		if (!plugin_suppresserrors) BBMessageBox(NULL, tempstring, szAppName, MB_OK|MB_SYSTEMMODAL);
		return 1;
	}

	//Create the details
	agenttype_winamp_details *details = new agenttype_winamp_details;
	a->agentdetails = (void *) details;

	//Is this the first?
	bool first = (agenttype_winamp_agents->first == NULL ? true : false);

	//Create a unique string to assign to this (just a number from a counter)
	wchar_t identifierstring[64];
	swprintf(identifierstring, 64, L"%ul", agenttype_winamp_counter);
	details->internal_identifier = new_string(identifierstring);

	//Add this to our internal tracking list
	agent *oldagent; //Unused, but we have to pass it
	list_add(agenttype_winamp_agents, details->internal_identifier, (void *) a, (void **) &oldagent);

	//Copy the code
	details->commandcode = commandcode;

	//IF this is the first, start the timer
	if (first)
	{
		SetTimer(agenttype_winamp_window, 0, 1000, agenttype_winamp_timercall);
	}

	//Increment the counter
	agenttype_winamp_counter++;

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_winamp_destroy(agent *a)
{
	if (a->agentdetails)
	{
		agenttype_winamp_details *details = (agenttype_winamp_details *) a->agentdetails;

		if (details->internal_identifier != NULL)
		{
			list_remove(agenttype_winamp_agents, details->internal_identifier);
			free_string(&details->internal_identifier);
		}

		delete (agenttype_winamp_details *) a->agentdetails;   
		a->agentdetails = NULL;
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_winamp_message(agent *a, int tokencount, char *tokens[])
{
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamp_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_winamp_details *details = (agenttype_winamp_details *) a->agentdetails;

	switch(notifytype)
	{
		case NOTIFY_CHANGE:
			//Find the Winamp window if necessary
			agenttype_winamp_getwinampwindow();
			if (!winamp_hwnd) return;

			//Send the message if we found the window
			PostMessage(winamp_hwnd, WM_COMMAND, details->commandcode, 0);
			break;
		case NOTIFY_SAVE_AGENT:
			//Write existance
			config_write(config_get_control_setagent_i(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, &details->commandcode));
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamppoller_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_winamp_details *details = (agenttype_winamp_details *) a->agentdetails;

	long length_seconds;
	long length_milliseconds;
	long newposition_milliseconds;

	switch(notifytype)
	{
		case NOTIFY_CHANGE:

			//Get the window
			agenttype_winamp_getwinampwindow();
			if (!winamp_hwnd) return;

			//Get the length of the track
			length_seconds = (long) SendMessage(winamp_hwnd, WM_USER, 1, 105);
			if (length_seconds == -1) return;
			length_milliseconds = length_seconds * 1000;

			//Get the percentage in
			newposition_milliseconds = (long) (length_milliseconds * *((double *)messagedata));
			//Set the milliseconds
			SendMessage(winamp_hwnd, WM_USER, newposition_milliseconds, 106);

			break;
		case NOTIFY_SAVE_AGENT:		
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, agenttype_winamp_pollingnames[details->commandcode]));
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_winamp_getdata(agent *a, int datatype)
{
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_winamppoller_getdata(agent *a, int datatype)
{
	agenttype_winamp_details *details = (agenttype_winamp_details *) a->agentdetails;

	switch(datatype)
	{
		case DATAFETCH_VALUE_TEXT:
			switch(details->commandcode)
			{
				case WINAMP_POLLINGTYPE_TITLE: return agenttype_winamp_currenttitle; break;
				case WINAMP_POLLINGTYPE_TIME_ELAPSED: return agenttype_winamp_timeelapsed; break;
				case WINAMP_POLLINGTYPE_TIME_REMAINING: return agenttype_winamp_timeremaining; break;
				case WINAMP_POLLINGTYPE_TIME_TOTAL: return agenttype_winamp_timetotal; break;
				case WINAMP_POLLINGTYPE_BITRATE: return agenttype_winamp_bitrate; break;
				default: return NULL; break;
			}
			break;

		case DATAFETCH_VALUE_SCALE:
			return &agenttype_winamp_trackposition; break;

		case DATAFETCH_VALUE_BOOL:
			return &agenttype_winamp_isplaying; break;
	}
	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamp_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	int set = -1;
	if (a)
	{
		agenttype_winamp_details *details = (agenttype_winamp_details *) a->agentdetails;
		set = details->commandcode;
	}

	for (int i = 0; i < winamp_actions_count; i++)
	{
		make_menuitem_bol(m, winamp_actions[i].string, config_getfull_control_setagent_i(c, action, L"Winamp", &winamp_actions[i].code), set == winamp_actions[i].code);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamppoller_bool_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamppoller_bool_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	make_menuitem_cmd(m, L"Track Is Playing", config_getfull_control_setagent_c(c, action, L"WinampBool", L"IsPlaying"));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamppoller_scale_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamppoller_scale_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	make_menuitem_cmd(m, L"Track Position", config_getfull_control_setagent_c(c, action, L"WinampScale", L"TrackPosition"));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamppoller_text_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamppoller_text_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	for (int i = WINAMP_POLLINGTYPE_LASTSCALE + 1; i < WINAMP_POLLINGTYPECOUNT; i++)
	{
		make_menuitem_cmd(m, agenttype_winamp_friendlynames[i], config_getfull_control_setagent_c(c, action, L"WinampText", agenttype_winamp_pollingnames[i]));
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamp_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
	make_menuitem_nop(m, L"No options available.");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamp_notifytype(int notifytype, void *messagedata)
{

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_event
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LRESULT CALLBACK agenttype_winamp_event(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_event
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool agenttype_winamp_getwinampwindow()
{
	if (!winamp_hwnd || !IsWindow(winamp_hwnd))
	{
		winamp_hwnd = FindWindow(L"Winamp v1.x",NULL); 
	}
	return (winamp_hwnd == NULL ? false : true);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_timercall
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VOID CALLBACK agenttype_winamp_timercall(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	//If there are agents left
	if (agenttype_winamp_agents->first != NULL)
	{
		agenttype_winamppoller_updatevalues();
	}
	else
	{
		KillTimer(hwnd, 0);
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_winamp_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_winamppoller_updatevalues()
{
		//This will need to be resized
		bool needsupdate[WINAMP_POLLINGTYPECOUNT];
		for (int i = 1; i < WINAMP_POLLINGTYPECOUNT; i++)
		{
			needsupdate[i] = false;
		}

		//Go through every agent type, see what needs updating
		listnode *currentnode;
		agent *currentagent;
		dolist(currentnode, agenttype_winamp_agents)
		{
			currentagent = (agent *) currentnode->value;
			needsupdate[((agenttype_winamp_details *) currentagent->agentdetails)->commandcode] = true;
		}

		//Make sure the window is there
		bool setdefaults = false;
		agenttype_winamp_getwinampwindow();
		if (!winamp_hwnd)
		{
			setdefaults = true;
		}
		else
		{	
				//case WINAMP_POLLINGTYPE_TIME_ELAPSED: return agenttype_winamp_timeelapsed; break;
				//case WINAMP_POLLINGTYPE_TIME_REMAINING: return agenttype_winamp_timeremaining; break;
				//case WINAMP_POLLINGTYPE_TIME_TOTAL: return agenttype_winamp_timetotal; break;			//If we need to update the position
			if (needsupdate[WINAMP_POLLINGTYPE_POSITION] == true
				|| needsupdate[WINAMP_POLLINGTYPE_TIME_ELAPSED]
				|| needsupdate[WINAMP_POLLINGTYPE_TIME_REMAINING]
				|| needsupdate[WINAMP_POLLINGTYPE_TIME_TOTAL]
				|| needsupdate[WINAMP_POLLINGTYPE_ISPLAYING]
				|| needsupdate[WINAMP_POLLINGTYPE_BITRATE])
			{				
				//Get the current winamp position and length
				long length_seconds = (long) SendMessage(winamp_hwnd, WM_USER, 1, 105);
				long currentmilliseconds = (long) SendMessage(winamp_hwnd, WM_USER, 0, 105);
				long bitrate=(long) SendMessage(winamp_hwnd, WM_USER,1,126);
				if (length_seconds == -1 || currentmilliseconds == -1)
				{
					setdefaults = true;
				}
				else
				{
					//It is playing
					agenttype_winamp_isplaying = true;

					//Calculate percentage
					double currentposition = currentmilliseconds / (length_seconds * 1000.0);
					if (currentposition > 1.0) currentposition = 1.0;
					agenttype_winamp_trackposition = currentposition;

					//Write text fields if necessary
					if (needsupdate[WINAMP_POLLINGTYPE_TIME_ELAPSED]) swprintf(agenttype_winamp_timeelapsed, 32, L"%d:%02d", currentmilliseconds / 60000, (currentmilliseconds / 1000) % 60);
					if (needsupdate[WINAMP_POLLINGTYPE_TIME_REMAINING]) swprintf(agenttype_winamp_timeremaining, 32, L"%d:%02d", (length_seconds - currentmilliseconds/1000)/60, (length_seconds - currentmilliseconds/1000) % 60);
					if (needsupdate[WINAMP_POLLINGTYPE_TIME_TOTAL]) swprintf(agenttype_winamp_timetotal, 32, L"%d:%02d", (length_seconds / 60), (length_seconds % 60));
					if (needsupdate[WINAMP_POLLINGTYPE_BITRATE]) swprintf(agenttype_winamp_bitrate, 32, L"%d", bitrate);
				}
			}

			//If we need to update the title
			if (needsupdate[WINAMP_POLLINGTYPE_TITLE] == true)
			{
				//Get the window text
				GetWindowText(winamp_hwnd, agenttype_winamp_titlebuffer, 1024);
				agenttype_winamp_currenttitle = agenttype_winamp_titlebuffer;

				//Strip off the "Winamp" at the end
				wchar_t *winamptitle = wcsstr(agenttype_winamp_titlebuffer, L" - Winamp");
				if (winamptitle != NULL) winamptitle[0] = L'\0';

				//See if there is a number number number dot space
				int i;
				for (i = 0; i < 5 && agenttype_winamp_currenttitle[i] >= '0' && agenttype_winamp_currenttitle[i] <= '9'; i++)
				{
					//Do nothing
				}
				//If the current character is a period, and the next is a space... trim off the number
				if (agenttype_winamp_currenttitle[i] == '.' && agenttype_winamp_currenttitle[i+1] == ' ')
				{
					agenttype_winamp_currenttitle = agenttype_winamp_titlebuffer + i + 2;
				}
			}
		}

		//If we need to set the defaults
		if (setdefaults)
		{
			agenttype_winamp_isplaying = false;
			agenttype_winamp_trackposition = 0.0;
			agenttype_winamp_trackposition = 0.0;
			wcscpy(agenttype_winamp_titlebuffer, L"");
			agenttype_winamp_currenttitle = agenttype_winamp_titlebuffer;
			wsprintf(agenttype_winamp_timeelapsed, L"");
			wsprintf(agenttype_winamp_timeremaining, L"");
			wsprintf(agenttype_winamp_timetotal, L"");
			wsprintf(agenttype_winamp_bitrate, L"");
		}

		//Go through every agent
		dolist(currentnode, agenttype_winamp_agents)
		{
			//Get the agent
			currentagent = (agent *) currentnode->value;
			control_notify(currentagent->controlptr, NOTIFY_NEEDUPDATE, NULL);
		}
}


/*=================================================*/
