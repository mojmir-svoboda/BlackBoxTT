/*===================================================

	AGENTTYPE_SYSTEMINFO CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
//Parent Include
#include "AgentType_SystemInfo.h"

//Includes
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "MessageMaster.h"
#include "ListMaster.h"

#ifndef SM_TABLETPC
	#define SM_TABLETPC	86
#endif
#ifndef SM_MEDIACENTER
	#define SM_MEDIACENTER	87
#endif
#ifndef SM_STARTER
	#define SM_STARTER	88
#endif
#ifndef SM_SERVERR2
	#define SM_SERVERR2	89
#endif
#ifndef VER_SUITE_WH_SERVER
	#define VER_SUITE_WH_SERVER 0x00008000
#endif

//Declare the function prototypes;
VOID CALLBACK agenttype_systeminfo_timercall(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK agenttype_systeminfo_event(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void agenttype_systeminfo_propogatenewvalues();
void agenttype_systeminfo_updatevalue(int monitor_type);
wchar_t * GetOSDetailInfo(wchar_t * os_string, size_t n, int type);

// Some windowing variables
HWND agenttype_systeminfo_window;
bool agenttype_systeminfo_windowclassregistered;

//Local primitives
unsigned long agenttype_systeminfo_counter;
const wchar_t agenttype_systeminfo_timerclass[] = L"BBInterfaceAgentSystemInfo";

//A list of this type of agent
list *agenttype_systeminfo_agents;
bool agenttype_systeminfo_hastimer = false;

//Declare the number of types
#define SYSTEMINFO_NUMTYPES 8

//Array of string types - must have SYSTEMINFO_NUMTYPES entries
enum SYSTEMINFO_TYPE
{
	SYSTEMINFO_TYPE_NONE = 0,
	SYSTEMINFO_TYPE_HOSTNAME,
	SYSTEMINFO_TYPE_USERNAME,
	SYSTEMINFO_TYPE_UPTIME,
	SYSTEMINFO_TYPE_OSNAME,
	SYSTEMINFO_TYPE_OSBUILDNUM,
	SYSTEMINFO_TYPE_BBVER,
	SYSTEMINFO_TYPE_STYLENAME
};

//Must match the enum ordering above! Must have SYSTEMINFO_NUMTYPES entries
const wchar_t *agenttype_systeminfo_types[] =
{
	L"None", // Unused
	L"HostName",
	L"UserName",
	L"Uptime",
	L"OSName",
	L"OSBuildNum",
	L"BBVer",
	L"StyleName"
};

//Must match the enum ordering above! Must have SYSTEMINFO_NUMTYPES entries
const wchar_t *agenttype_systeminfo_friendlytypes[] =
{
	L"None", // Unused
	L"Host Name",
	L"User Name",
	L"Uptime",
	L"OS Name",
	L"OS Build Number",
	L"BB Version",
	L"Style Name"
};

//Must be the same size as the above array and enum
wchar_t agenttype_systeminfo_values[SYSTEMINFO_NUMTYPES][256];

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//controltype_button_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systeminfo_startup()
{
	//Create the list
	agenttype_systeminfo_agents = list_create();

	//Register the window class
	agenttype_systeminfo_windowclassregistered = false;
	if (window_helper_register(agenttype_systeminfo_timerclass, &agenttype_systeminfo_event))
	{
		//Couldn't register the window
		BBMessageBox(NULL, L"failed on register class", L"test", MB_OK);
		return 1;
	}
	agenttype_systeminfo_windowclassregistered = true;

	//Create the window
	agenttype_systeminfo_window = window_helper_create(agenttype_systeminfo_timerclass);
	if (!agenttype_systeminfo_window)
	{
		//Couldn't create the window
		BBMessageBox(NULL, L"failed on window", "test", MB_OK);
		return 1;
	}
	
	//initialize
	for (int i = 1; i < SYSTEMINFO_NUMTYPES; i++){
		agenttype_systeminfo_updatevalue(i);
	}

	//If we got this far, we can successfully use this function
	//Register this type with the AgentMaster
	agent_registertype(
		L"System Information",                   //Friendly name of agent type
		L"SystemInfo",                    //Name of agent type
		CONTROL_FORMAT_TEXT,				//Control type
		false,
		&agenttype_systeminfo_create,
		&agenttype_systeminfo_destroy,
		&agenttype_systeminfo_message,
		&agenttype_systeminfo_notify,
		&agenttype_systeminfo_getdata,
		&agenttype_systeminfo_menu_set,
		&agenttype_systeminfo_menu_context,
		&agenttype_systeminfo_notifytype
		);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systeminfo_shutdown()
{
	if(agenttype_systeminfo_hastimer){
		agenttype_systeminfo_hastimer = false;
		KillTimer(agenttype_systeminfo_window, 0);
	}
	//Destroy the internal tracking list
	if (agenttype_systeminfo_agents) list_destroy(agenttype_systeminfo_agents);
	
	//Destroy the window
	if (agenttype_systeminfo_window) window_helper_destroy(agenttype_systeminfo_window);

	//Unregister the window class
	if (agenttype_systeminfo_windowclassregistered) window_helper_unregister(agenttype_systeminfo_timerclass);


	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systeminfo_create(agent *a, wchar_t *parameterstring)
{


	if (0 == * parameterstring)
		return 2; // no param, no agent

	//Find the monitor type
	int monitor_type = SYSTEMINFO_TYPE_NONE;
	for (int i = 1; i < SYSTEMINFO_NUMTYPES; i++)
	{
		if (_wcsicmp(agenttype_systeminfo_types[i], parameterstring) == 0)
		{
			monitor_type = i;
			break;
		}
	}

	//If we didn't find a correct monitor type
	if (monitor_type == SYSTEMINFO_TYPE_NONE)
	{
		//On an error
		if (!plugin_suppresserrors)
		{
			wchar_t buffer[1000];
			swprintf(buffer,1000, L"There was an error setting the System Information agent:\n\nType \"%s\" is not a valid type.", parameterstring);
			BBMessageBox(NULL, buffer, szAppName, MB_OK|MB_SYSTEMMODAL);
		}
		return 1;
	}
	
	//Create the details
	agenttype_systeminfo_details *details = new agenttype_systeminfo_details;
	details->monitor_type = monitor_type;
	
	//Create a unique string to assign to this (just a number from a counter)
	wchar_t identifierstring[64];
	swprintf(identifierstring, L"%ul", agenttype_systeminfo_counter);
	details->internal_identifier = new_string(identifierstring);

	//Set the details
	a->agentdetails = (void *)details;

	//Add this to our internal tracking list ( need update value only )
	if(details->monitor_type == SYSTEMINFO_TYPE_UPTIME || details->monitor_type == SYSTEMINFO_TYPE_STYLENAME){
		agent *oldagent;
		list_add(agenttype_systeminfo_agents, details->internal_identifier, (void *) a, (void **) &oldagent);
		agenttype_systeminfo_updatevalue(details->monitor_type);
		//Start Timer 
		if (!agenttype_systeminfo_hastimer)
		{
			SetTimer(agenttype_systeminfo_window, 0, 1000, agenttype_systeminfo_timercall);
			agenttype_systeminfo_hastimer = true;
		}
	}
	//Increment the counter
	agenttype_systeminfo_counter++;


	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systeminfo_destroy(agent *a)
{
	if (a->agentdetails)
	{
		agenttype_systeminfo_details *details = (agenttype_systeminfo_details *) a->agentdetails;
		//Delete from the tracking list
		if(details->monitor_type == SYSTEMINFO_TYPE_UPTIME || details->monitor_type == SYSTEMINFO_TYPE_STYLENAME){
			list_remove(agenttype_systeminfo_agents, details->internal_identifier);
		}
		//Free the string
		free_string(&details->internal_identifier);

		delete details;
		a->agentdetails = NULL;
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systeminfo_message(agent *a, int tokencount, wchar_t *tokens[])
{
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systeminfo_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_systeminfo_details *details = (agenttype_systeminfo_details *) a->agentdetails;

	switch(notifytype)
	{
		case NOTIFY_CHANGE:
			control_notify(a->controlptr, NOTIFY_NEEDUPDATE, NULL);
			break;

		case NOTIFY_SAVE_AGENT:
			//Write existance
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, agenttype_systeminfo_types[details->monitor_type]));
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_systeminfo_getdata(agent *a, int datatype)
{
	//Get the agent details
	agenttype_systeminfo_details *details = (agenttype_systeminfo_details *) a->agentdetails;

	return &agenttype_systeminfo_values[details->monitor_type];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systeminfo_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	//Add a menu item for every type
	for (int i = 1; i < SYSTEMINFO_NUMTYPES; i++)
	{
		make_menuitem_cmd(m, agenttype_systeminfo_friendlytypes[i], config_getfull_control_setagent_c(c, action, L"SystemInfo", agenttype_systeminfo_types[i]));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systeminfo_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
	make_menuitem_nop(m, L"No options available.");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systeminfo_notifytype(int notifytype, void *messagedata)
{

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_timercall
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VOID CALLBACK agenttype_systeminfo_timercall(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	//If there are agents left
	if (agenttype_systeminfo_agents->first != NULL)
	{
		agenttype_systeminfo_propogatenewvalues(); 
	}
	else
	{
		agenttype_systeminfo_hastimer = false;
		KillTimer(hwnd, 0);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_event
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LRESULT CALLBACK agenttype_systeminfo_event(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_propogatenewvalues
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systeminfo_propogatenewvalues()
{
	//Declare variables
	agent *currentagent;

	//Go through every agent
	listnode *currentnode;
	dolist(currentnode, agenttype_systeminfo_agents)
	{
		//Get the agent
		currentagent = (agent *) currentnode->value;

		//Get the monitor type
		int monitor_type = ((agenttype_systeminfo_details *) (currentagent->agentdetails))->monitor_type;

		//Calculate a new value for this type
		
		agenttype_systeminfo_updatevalue(monitor_type);
		control_notify(currentagent->controlptr, NOTIFY_NEEDUPDATE, NULL);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systeminfo_updatevalue
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systeminfo_updatevalue(int monitor_type)
{
	wchar_t tmpstr[256];
	DWORD len = 256;
	DWORD time;
	switch(monitor_type)
	{
		case SYSTEMINFO_TYPE_HOSTNAME:
			if(GetComputerName(tmpstr,&len))
				wcsncpy(agenttype_systeminfo_values[monitor_type],tmpstr,256);
			break;
		case SYSTEMINFO_TYPE_USERNAME:
			if(GetUserName(tmpstr,&len))
				wcsncpy(agenttype_systeminfo_values[monitor_type],tmpstr,256);
			break;
		case SYSTEMINFO_TYPE_UPTIME:
			time = GetTickCount();
			swprintf(agenttype_systeminfo_values[monitor_type],256, L"%dd %2dh %2dm %2ds",time/86400000,(time/3600000)%24,(time/60000)%60,(time/1000)%60);
			break;
		case SYSTEMINFO_TYPE_OSNAME:
		case SYSTEMINFO_TYPE_OSBUILDNUM:
			GetOSDetailInfo(agenttype_systeminfo_values[monitor_type], 256, monitor_type);
			break;
		case SYSTEMINFO_TYPE_BBVER:
			wcsncpy(agenttype_systeminfo_values[monitor_type],GetBBVersion(),256);
			break;
		case SYSTEMINFO_TYPE_STYLENAME:
			wcsncpy(agenttype_systeminfo_values[monitor_type], ReadString(stylePath(), L"style.name:", L"no name"), 256);
			break;
	}

	return;
}

//-------------------------------------------------------------
//GetOSDetailInfo
//-------------------------------------------------------------
wchar_t * GetOSDetailInfo(wchar_t *os_string, size_t n, int type){
	OSVERSIONINFOEX osver;
	BOOL success_getver;
	typedef BOOL (WINAPI *PFUNC_GetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
	PFUNC_GetProductInfo pGetProductInfo = NULL;
	DWORD dwReturnedProductType = 0;
	ZeroMemory(&osver,sizeof(OSVERSIONINFOEX));
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!(success_getver = GetVersionEx((OSVERSIONINFO *) &osver)))
	{
		//OSVERSIONINFOEX does not support
		osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *) &osver);
	}
	if(type == SYSTEMINFO_TYPE_OSBUILDNUM){
		if(osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS){
			swprintf(os_string, n, L"%d",LOWORD(osver.dwBuildNumber));
		}else{
			swprintf(os_string, n, L"%d",osver.dwBuildNumber);
		}
		return os_string;
	}
	if(osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		switch(osver.dwMinorVersion){
			case 0: 
				if( LOWORD(osver.dwBuildNumber) == 1111){
					wcscpy(os_string,L"Windows 95 OSR2");
				}else if( LOWORD(osver.dwBuildNumber) >= 1214){
					wcscpy(os_string,L"Windows 95 OSR2.5");
				}else if( LOWORD(osver.dwBuildNumber) >= 1212){
					wcscpy(os_string,L"Windows 95 OSR2.1");
				}else{
					wcscpy(os_string,L"Windows 95");
				}
				break;
			case 10:
				if( LOWORD(osver.dwBuildNumber) >= 2222){
					wcscpy(os_string,L"Windows 98 SE");
				}else if( LOWORD(osver.dwBuildNumber) >= 2000){
					wcscpy(os_string,L"Windows 98 SP1");
				}else{
					wcscpy(os_string,L"Windows 98");
				}
				break;
			case 90:
				wcscpy(os_string,L"Windows Me");
				break;
			default:
				wcscpy(os_string,L"Windows 9x");
				break;
		}
	}
	else if(osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if(osver.dwMajorVersion == 3){
			swprintf(os_string, n, L"Windows NT3.%d",osver.dwMinorVersion);
		}else if(osver.dwMajorVersion == 4){
			if(success_getver){
				if(osver.wProductType == VER_NT_WORKSTATION){
					wcscpy(os_string, L"Windows NT Workstation 4.0");
				}else if(osver.wProductType == VER_NT_SERVER){
					wcscpy(os_string, L"Windows NT Server 4.0");
				}else{
					wcscpy(os_string, L"Windows NT 4.0");
				}
				if(osver.wSuiteMask & VER_SUITE_ENTERPRISE){
					wcscat(os_string, L" Enterprise Edition");
				}
			}
			else{
				wcscpy(os_string,L"Windows NT 4.0");
			}
		}else if(osver.dwMajorVersion == 5){
			if(osver.dwMinorVersion == 0){ // Win2k
				if(success_getver){
					if(osver.wProductType == VER_NT_WORKSTATION){
						wcscpy(os_string, L"Windows 2000 Professional");
					}else if(osver.wProductType == VER_NT_SERVER){
						if(osver.wSuiteMask & VER_SUITE_ENTERPRISE){
							wcscpy(os_string, L"Windows 2000 Advanced Server");
						}else if(osver.wSuiteMask & VER_SUITE_DATACENTER){
							wcscpy(os_string, L"Windows 2000 Datacenter Server");
						}else{
							wcscpy(os_string, L"Windows 2000 Server");
						}	
					}else{
						wcscpy(os_string,L"Windows 2000");
					}
				}else{
					wcscpy(os_string,L"Windows 2000");
				}
			}else if(osver.dwMinorVersion == 1){ // WinXP
				if(success_getver){
					 if(GetSystemMetrics(SM_MEDIACENTER)){
						 wcscpy(os_string,L"Windows XP Media Center Edition");
					 }else if(GetSystemMetrics(SM_TABLETPC)){
						 wcscpy(os_string,L"Windows XP Tablet PC Edition");
					 }else if(GetSystemMetrics(SM_STARTER)){
						 wcscpy(os_string,L"Windows XP Starter Edition");
					 }else if(osver.wSuiteMask & VER_SUITE_PERSONAL){
						 wcscpy(os_string,L"Windows XP Home Edition");
					 }else{
						 wcscpy(os_string,L"Windows XP Professional");
					 }
				}else{
					wcscpy(os_string,L"Windows XP");
				}
			}else if(osver.dwMinorVersion == 2){ //WinServer2003
				if(success_getver){
					wchar_t r2[4];
					wcscpy(r2, GetSystemMetrics(SM_SERVERR2)? L" R2": L"");
					if(osver.wProductType == VER_NT_WORKSTATION){
						wcscpy(os_string,L"Windows XP Professional x64 Edition");	
					}else if(osver.wSuiteMask & VER_SUITE_ENTERPRISE){
						swprintf(os_string,n,L"Windows Server 2003%s Enterprise Edition",r2);
					}else if(osver.wSuiteMask & VER_SUITE_DATACENTER){
						swprintf(os_string,n,L"Windows Server 2003%s Datacenter Edition",r2);
					}else if(osver.wSuiteMask & VER_SUITE_BLADE){
						swprintf(os_string,n,L"Windows Server 2003%s Web Edition",r2);
					}else if(osver.wSuiteMask & VER_SUITE_COMPUTE_SERVER){
						swprintf(os_string,n,L"Windows Server 2003%s Compute Cluster Edition",r2);
					}else if(osver.wSuiteMask & VER_SUITE_STORAGE_SERVER){
						swprintf(os_string,n,L"Windows Storage Server 2003%s",r2);
					}else if(osver.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED){
						swprintf(os_string,n,L"Microsoft Small Business Server 2003%s",r2);
					}else if(osver.wSuiteMask & VER_SUITE_WH_SERVER){
						wcscpy(os_string,L"Windows Home Server");
					}else{
						swprintf(os_string,n,L"Windows Server 2003%s Standard Edition",r2);
					}
				}else{
					wcscpy(os_string,L"Windows Server 2003");
				}
			}
		}else if(osver.dwMajorVersion == 6){
			if(success_getver){
				pGetProductInfo = (PFUNC_GetProductInfo)::GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetProductInfo");
				if(pGetProductInfo && pGetProductInfo(osver.dwMajorVersion,osver.dwMinorVersion,osver.wServicePackMajor,osver.wServicePackMinor,&dwReturnedProductType)){
					switch(dwReturnedProductType){
						case 0x00000006://PRODUCT_BUSINESS
						case 0x00000010://PRODUCT_BUSINESS
							wcscpy(os_string,L"Windows Vista Business");
							break;
						case 0x00000012://PRODUCT_CLUSTER_SERVER
							wcscpy(os_string,L"Cluster Server Edition");
							break;
						case 0x00000008://PRODUCT_DATACENTER_SERVER
							wcscpy(os_string,L"Datacenter Edition (full installation)");
							break;
						case 0x0000000C://PRODUCT_DATACENTER_SERVER_CORE
							wcscpy(os_string,L"Datacenter Edition (core installation)");
							break;
						case 0x00000004://PRODUCT_ENTERPRISE
						case 0x0000001B://PRODUCT_ENTERPRISE
							wcscpy(os_string,L"Windows Vista Enterprise");
							break;
						case 0x0000000A://PRODUCT_ENTERPRISE_SERVER
							wcscpy(os_string,L"Server Enterprise Edition (full installation)");
							break;
						case 0x0000000E://PRODUCT_ENTERPRISE_SERVER_CORE
							wcscpy(os_string,L"Server Enterprise Edition (core installation)");
							break;
						case 0x0000000F://PRODUCT_ENTERPRISE_SERVER_IA64
							wcscpy(os_string,L"Server Enterprise Edition for Itanium-based Systems");
							break;
						case 0x00000002://PRODUCT_HOME_BASIC
						case 0x00000005://PRODUCT_HOME_BASIC_N
							wcscpy(os_string,L"Windows Vista Home Basic");
							break;
						case 0x00000003://PRODUCT_HOME_PREMIUM
						case 0x0000001A://PRODUCT_HOME_PREMIUM_N
							wcscpy(os_string,L"Windows Vista Home Premium");
							break;
						case 0x00000013://PRODUCT_HOME_SERVER
							wcscpy(os_string,L"Home Server");
							break;
						case 0x00000018://PRODUCT_SERVER_FOR_SMALLBUSINESS
							wcscpy(os_string,L"Server for Small Business Edition");
							break;
						case 0x00000009://PRODUCT_SMALLBUSINESS_SERVER
							wcscpy(os_string,L"Small Business Server");
							break;
						case 0x00000019://PRODUCT_SMALLBUSINESS_SERVER_PREMIUM
							wcscpy(os_string,L"Small Business Server Premium Edition");
							break;
						case 0x00000007://PRODUCT_STANDARD_SERVER
							wcscpy(os_string,L"Server Standard Edition (full installation)");
							break;
						case 0x0000000D://PRODUCT_STANDARD_SERVER_CORE
							wcscpy(os_string,L"Server Standard Edition (core installation)");
							break;
						case 0x0000000B://PRODUCT_STARTER
							wcscpy(os_string,L"Windows Vista Starter");
							break;
						case 0x00000017://PRODUCT_STORAGE_ENTERPRISE_SERVER
							wcscpy(os_string,L"Storage Server Enterprise Edition");
							break;
						case 0x00000014://PRODUCT_STORAGE_EXPRESS_SERVER
							wcscpy(os_string,L"Storage Server Express Edition");
							break;
						case 0x00000015://PRODUCT_STORAGE_STANDARD_SERVER
							wcscpy(os_string,L"Storage Server Standard Edition");
							break;
						case 0x00000016://PRODUCT_STORAGE_WORKGROUP_SERVER
							wcscpy(os_string,L"Storage Server Workgroup Edition");
							break;
						case 0x00000001://PRODUCT_ULTIMATE
						case 0x0000001C://PRODUCT_ULTIMATE_N
							wcscpy(os_string,L"Windows Vista Ultimate");
							break;
						case 0x00000011://PRODUCT_WEB_SERVER
							wcscpy(os_string,L"Web Server Edition");
							break;
						default:
							wcscpy(os_string,L"Windows Vista");
							break;
					}//end switch
				}// end if(pGetProductInfo ... )
				else
				{
					wcscpy(os_string,L"Windows Vista");
				}
			}// end if(success_getver)
			else
			{
				wcscpy(os_string,L"Windows Vista");
			}
		}//end if vista
		if(success_getver && osver.wServicePackMajor){
			//Service Pack Check
			if(osver.wServicePackMinor){
				swprintf(os_string,n,L"%s SP%d.%d",os_string,osver.wServicePackMajor,osver.wServicePackMinor);
			}else{
				swprintf(os_string,n,L"%s SP%d",os_string,osver.wServicePackMajor);
			}
		}	
	}//end if(WinNT)
	return os_string;
}




					

