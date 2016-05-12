#include <blackbox/plugin/bb.h>
#include <windows.h>
#include "AgentType_SystemMonitor.h"
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "MessageMaster.h"
#include "ListMaster.h"
#include <cstring>
#include <string>
#include <vector>
#include "utils/core_load.h"
#include <blackbox/worker.h>

VOID CALLBACK agenttype_systemmonitor_timercall(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK agenttype_systemmonitor_event(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void agenttype_systemmonitor_propogatenewvalues();

struct SystemMonitor
{
	HWND m_agenttype_systemmonitor_window;
	bool m_agenttype_systemmonitor_windowclassregistered;
	HMODULE m_agenttype_systemmonitor_ntdllmodule;

	//The function call to get system information
	agenttype_systemmonitor_NtQuerySystemInformation m_agenttype_systemmonitor_ntquerysysteminformation;

	double m_agenttype_systemmonitor_number_processors;
	long double m_agenttype_systemmonitor_last_system_time;
	long double m_agenttype_systemmonitor_last_idle_time;
	unsigned long m_agenttype_systemmonitor_counter;
	std::string m_agenttype_systemmonitor_timerclass;

	list * m_agenttype_systemmonitor_agents; //A list of this type of agent
	bool m_agenttype_systemmonitor_hastimer;
	CoreLoad m_coreLoad;

	SystemMonitor ()
		: m_agenttype_systemmonitor_windowclassregistered(false)
		, m_agenttype_systemmonitor_number_processors(0.0)
		, m_agenttype_systemmonitor_last_system_time(0.0)
		, m_agenttype_systemmonitor_last_idle_time(0.0)
		, m_agenttype_systemmonitor_counter(0)
		, m_agenttype_systemmonitor_timerclass("BBInterfaceAgentSystemMon")
		, m_agenttype_systemmonitor_agents(nullptr)
		, m_agenttype_systemmonitor_hastimer(false)
	{ }

	bool Init ();
	bool Done ();
};

SystemMonitor * g_sysMon = nullptr;

//Array of string types - must have SYSTEMMONITOR_NUMTYPES entries
enum SYSTEMMONITOR_TYPE
{
	SYSTEMMONITOR_TYPE_NONE = 0,
	SYSTEMMONITOR_TYPE_CPUUSAGE,
	SYSTEMMONITOR_TYPE_PHYSMEMFREE,
	SYSTEMMONITOR_TYPE_PHYSMEMUSED,
	SYSTEMMONITOR_TYPE_VIRTMEMFREE,
	SYSTEMMONITOR_TYPE_VIRTMEMUSED,
	SYSTEMMONITOR_TYPE_PAGEFILEFREE,
	SYSTEMMONITOR_TYPE_PAGEFILEUSED,
	SYSTEMMONITOR_TYPE_BATTERYPOWER,
	SYSTEMMONITOR_TYPE_COREUSAGE,
	SYSTEMMONITOR_NUMTYPES
};

//Must match the enum ordering above! Must have SYSTEMMONITOR_NUMTYPES entries
const char *agenttype_systemmonitor_types[] =
{
	"None", // Unused
	"CPUUsage",
	"PhysicalMemoryFree",
	"PhysicalMemoryUsed",
	"VirtualMemoryFree",
	"VirtualMemoryUsed",
	"PageFileFree",
	"PageFileUsed",
	"BatteryPower",
	"CoreUsage"
};

//Must match the enum ordering above! Must have SYSTEMMONITOR_NUMTYPES entries
const char *agenttype_systemmonitor_friendlytypes[] =
{
	"None", // Unused
	"CPU Usage",
	"Physical Memory Free",
	"Physical Memory Used",
	"Virtual Memory Free",
	"Virtual Memory Used",
	"Page File Free",
	"Page File Used",
	"Battery Power",
	"Core Usages",
	"Core Usage"
};

bool is2kxp = false;

struct agenttype_systemmonitor_details
{
	char const * internal_identifier;
	SYSTEMMONITOR_TYPE monitor_type;
	size_t monitor_index;
	int m_int_arg;
};
void agenttype_systemmonitor_updatevalue(agenttype_systemmonitor_details const * details);

struct SYSTEM_BASIC_INFORMATION
{
	DWORD dwUnknown1;
	ULONG uKeMaximumIncrement;
	ULONG uPageSize;
	ULONG uMmNumberOfPhysicalPages;
	ULONG uMmLowestPhysicalPage;
	ULONG uMmHighestPhysicalPage;
	ULONG uAllocationGranularity;
	PVOID pLowestUserAddress;
	PVOID pMmHighestUserAddress;
	ULONG uKeActiveProcessors;
	BYTE bKeNumberProcessors;
	BYTE bUnknown2;
	WORD wUnknown3;
};

struct SYSTEM_PERFORMANCE_INFORMATION
{
	LARGE_INTEGER liIdleTime;
	DWORD dwSpare[76];
};

struct SYSTEM_TIME_INFORMATION
{
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;
};


struct MonitorInfo
{
	SYSTEMMONITOR_TYPE m_type;
	double m_value;
	double m_prev_value;
	std::string m_str_value;

	MonitorInfo () : m_type(SYSTEMMONITOR_TYPE_NONE), m_value(0.0), m_prev_value(0.0), m_str_value() { }
	MonitorInfo (SYSTEMMONITOR_TYPE t) : m_type(t), m_value(0.0), m_prev_value(0.0), m_str_value() { }
};

std::vector<MonitorInfo> g_monitors;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//controltype_button_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systemmonitor_startup ()
{
	if (g_sysMon)
	{
		g_sysMon->Done();
		delete g_sysMon;
		g_sysMon = nullptr;
	}
	
	g_sysMon = new SystemMonitor();
	if (g_sysMon->Init())
	{
		//If we got this far, we can successfully use this function
		//Register this type with the AgentMaster
		agent_registertype(
			"System Monitor",                   //Friendly name of agent type
			"SystemMonitor",                    //Name of agent type
			CONTROL_FORMAT_SCALE | CONTROL_FORMAT_TEXT,				//Control type
			false,
			&agenttype_systemmonitor_create,
			&agenttype_systemmonitor_destroy,
			&agenttype_systemmonitor_message,
			&agenttype_systemmonitor_notify,
			&agenttype_systemmonitor_getdata,
			&agenttype_systemmonitor_menu_set,
			&agenttype_systemmonitor_menu_context,
			&agenttype_systemmonitor_notifytype
			);
	}
	return 0;
}

bool SystemMonitor::Init ()
{
	//This code stolen from the bb4win SDK example
	OSVERSIONINFO osvinfo;
	ZeroMemory(&osvinfo, sizeof(osvinfo));
	osvinfo.dwOSVersionInfoSize = sizeof (osvinfo);
	GetVersionEx(&osvinfo);
	is2kxp = osvinfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

	//Create the list
	m_agenttype_systemmonitor_agents = list_create();

	//Register the window class
	m_agenttype_systemmonitor_windowclassregistered = false;
	if (window_helper_register(m_agenttype_systemmonitor_timerclass.c_str(), &agenttype_systemmonitor_event))
	{
		//Couldn't register the window
		BBMessageBox(NULL, "failed on register class", "test", MB_OK);
		return false;
	}
	m_agenttype_systemmonitor_windowclassregistered = true;

	//Create the window
	m_agenttype_systemmonitor_window = window_helper_create(m_agenttype_systemmonitor_timerclass.c_str());
	if (!m_agenttype_systemmonitor_window)
	{
		//Couldn't create the window
		BBMessageBox(NULL, "failed on window", "test", MB_OK);
		return false;
	}

	//Load the library
	if (is2kxp)
	{
		m_agenttype_systemmonitor_ntdllmodule = NULL;
		m_agenttype_systemmonitor_ntdllmodule = LoadLibrary("ntdll.dll");

		//Check to make sure it loaded properly
		if (m_agenttype_systemmonitor_ntdllmodule == NULL)
		{
			//We couldn't load the NTDLL library
			//Return immediately
			return false;
			BBMessageBox(NULL, "failed on ntdll", "test", MB_OK);
		}

		//Get the NtQuerySystemInformation function
		m_agenttype_systemmonitor_ntquerysysteminformation = NULL;
		m_agenttype_systemmonitor_ntquerysysteminformation = (agenttype_systemmonitor_NtQuerySystemInformation)GetProcAddress(m_agenttype_systemmonitor_ntdllmodule, "NtQuerySystemInformation");

		//Check to make sure we could get the function
		if (m_agenttype_systemmonitor_ntquerysysteminformation == NULL)
		{
			//Error - couldn't get the function call
			FreeLibrary(m_agenttype_systemmonitor_ntdllmodule);
			BBMessageBox(NULL, "failed on get proc address", "test", MB_OK);
			return false;
		}

		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		//Record the number of processors
		m_agenttype_systemmonitor_number_processors = sysinfo.dwNumberOfProcessors;

		//If it is less than 1 or more than 64... assume an error (I don't think any super clusters are running BBI)
		if (m_agenttype_systemmonitor_number_processors < 1 || m_agenttype_systemmonitor_number_processors > 64)
		{
			FreeLibrary(m_agenttype_systemmonitor_ntdllmodule);
			BBMessageBox(NULL, "failed on number of processors", "test", MB_OK);
			return false;
		}
	}
	return true;
}

bool SystemMonitor::Done()
{
	if (m_agenttype_systemmonitor_hastimer)
	{
		m_agenttype_systemmonitor_hastimer = false;
		KillTimer(m_agenttype_systemmonitor_window, 0);
	}
	//Destroy the internal tracking list
	if (m_agenttype_systemmonitor_agents)
		list_destroy(m_agenttype_systemmonitor_agents);
	//Destroy the window
	if (m_agenttype_systemmonitor_window)
		window_helper_destroy(m_agenttype_systemmonitor_window);

	//Unregister the window class
	if (m_agenttype_systemmonitor_windowclassregistered)
		window_helper_unregister(m_agenttype_systemmonitor_timerclass.c_str());

	//If we have the library, free it
	if (m_agenttype_systemmonitor_ntdllmodule != NULL)
	{
		FreeLibrary(m_agenttype_systemmonitor_ntdllmodule);
	}
	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systemmonitor_shutdown()
{
	if (g_sysMon)
	{
		g_sysMon->Done();
		delete g_sysMon;
		g_sysMon = nullptr;
	}
	return 0;
}

SYSTEMMONITOR_TYPE findMonitorType (char const * str)
{
	//Find the monitor type
	int monitor_type = SYSTEMMONITOR_TYPE_NONE;
	for (int i = 1; i < SYSTEMMONITOR_NUMTYPES; i++)
	{
		std::string const t = agenttype_systemmonitor_types[i]; // hm, inefficient
		std::size_t found = std::string(str).find(t);
		if (found != std::string::npos)
		{
			monitor_type = i;
			break;
		}
	}
	return static_cast<SYSTEMMONITOR_TYPE>(monitor_type);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systemmonitor_create (agent * a, char * parameterstring)
{
	if (0 == * parameterstring)
		return 2; // no param, no agent

	std::string arg(parameterstring);
	SYSTEMMONITOR_TYPE const monitor_type = findMonitorType(parameterstring);

	//If we didn't find a correct monitor type
	if (monitor_type == SYSTEMMONITOR_TYPE_NONE)
	{
		//On an error
		if (!plugin_suppresserrors)
		{
			char buffer[1024];
			_snprintf_s(buffer, 1024, "There was an error setting the System Monitor agent:\n\nType \"%s\" is not a valid type.", parameterstring);
			BBMessageBox(NULL, buffer, szAppName, MB_OK|MB_SYSTEMMODAL);
		}
		return 1;
	}

	//Create the details
	agenttype_systemmonitor_details *details = new agenttype_systemmonitor_details;
	details->monitor_type = monitor_type;

	if (monitor_type == SYSTEMMONITOR_TYPE_COREUSAGE)
	{
		//parse for core number
		std::string core_arg = arg.substr(strlen(agenttype_systemmonitor_types[SYSTEMMONITOR_TYPE_COREUSAGE]));
		details->m_int_arg = atoi(core_arg.c_str()); //@TODO: err handling
	}

	//Create a unique string to assign to this (just a number from a counter)
	char identifierstring[64];
	sprintf(identifierstring, "%ul", g_sysMon->m_agenttype_systemmonitor_counter);
	details->internal_identifier = new_string(identifierstring);

	//Set the details
	a->agentdetails = static_cast<void *>(details);

	//Add this to our internal tracking list
	agent * oldagent = 0; //Unused, but we have to pass it
	list_add(g_sysMon->m_agenttype_systemmonitor_agents, details->internal_identifier, (void *)a, (void **)&oldagent);
	g_monitors.push_back(MonitorInfo(monitor_type));
	details->monitor_index = g_monitors.size() - 1;

	//Increment the counter
	g_sysMon->m_agenttype_systemmonitor_counter++;

	if (!g_sysMon->m_agenttype_systemmonitor_hastimer)
	{
		SetTimer(g_sysMon->m_agenttype_systemmonitor_window, 0, 1000, agenttype_systemmonitor_timercall);
		g_sysMon->m_agenttype_systemmonitor_hastimer = true;
	}
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systemmonitor_destroy (agent * a)
{
	if (a->agentdetails)
	{
		agenttype_systemmonitor_details * const details = static_cast<agenttype_systemmonitor_details *>(a->agentdetails);

		// Delete from the tracking list
		list_remove(g_sysMon->m_agenttype_systemmonitor_agents, details->internal_identifier);

		delete details;
		a->agentdetails = nullptr;
	}
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_systemmonitor_message (agent * a, int tokencount, char * tokens[] ) { return 0; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systemmonitor_notify (agent * a, int notifytype, void * messagedata)
{
	//Get the agent details
	agenttype_systemmonitor_details const * const details = static_cast<agenttype_systemmonitor_details const *>(a->agentdetails);

	switch (notifytype)
	{
		case NOTIFY_CHANGE:
			control_notify(a->controlptr, NOTIFY_NEEDUPDATE, NULL);
			break;

		case NOTIFY_SAVE_AGENT:
		{
			//Write existance
			std::string type(agenttype_systemmonitor_types[details->monitor_type]);
			if (details->monitor_type == SYSTEMMONITOR_TYPE_COREUSAGE)
				type += std::to_string(details->m_int_arg);
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, type.c_str()));
			break;
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void * agenttype_systemmonitor_getdata (agent * a, int datatype)
{
	//Get the agent details
	agenttype_systemmonitor_details const * const details = static_cast<agenttype_systemmonitor_details const *>(a->agentdetails);
	size_t const monitor_index = details->monitor_index;

	switch (datatype)
	{
		case DATAFETCH_VALUE_SCALE: return &g_monitors[monitor_index].m_value;
		case DATAFETCH_VALUE_TEXT: return const_cast<char *>(g_monitors[monitor_index].m_str_value.c_str()); // @TODO unconst_cast
	}

	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systemmonitor_menu_set (Menu * m, control * c, agent * a,  char * action, int controlformat)
{
	//Add a menu item for every type (except none and core usage)
	for (int i = 1; i < SYSTEMMONITOR_TYPE_COREUSAGE; i++)
	{
		make_menuitem_cmd(m, agenttype_systemmonitor_friendlytypes[i], config_getfull_control_setagent_c(c, action, "SystemMonitor", agenttype_systemmonitor_types[i]));
	}

	//make menu for cores
	for (size_t i = 0, ie = g_sysMon->m_coreLoad.GetCoreCount(); i < ie; ++i)
	{
		std::string friendly(agenttype_systemmonitor_friendlytypes[SYSTEMMONITOR_TYPE_COREUSAGE]);
		friendly += std::to_string(i);
		std::string type(agenttype_systemmonitor_types[SYSTEMMONITOR_TYPE_COREUSAGE]);
		type += std::to_string(i);

		make_menuitem_cmd(m, friendly.c_str(), config_getfull_control_setagent_c(c, action, "SystemMonitor", type.c_str()));
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systemmonitor_menu_context (Menu * m, agent * a)
{
	make_menuitem_nop(m, "No options available.");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systemmonitor_notifytype (int notifytype, void * messagedata) { }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_timercall
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VOID CALLBACK agenttype_systemmonitor_timercall (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	//If there are agents left
	if (g_sysMon->m_agenttype_systemmonitor_agents->first != NULL)
	{
		agenttype_systemmonitor_propogatenewvalues();
	}
	else
	{
		g_sysMon->m_agenttype_systemmonitor_hastimer = false;
		KillTimer(hwnd, 0);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_event
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LRESULT CALLBACK agenttype_systemmonitor_event (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_propogatenewvalues
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systemmonitor_propogatenewvalues ()
{
	// Reset all values to unknown
	for (int i = 0; i < g_monitors.size(); i++)
	{
		MonitorInfo & mi = g_monitors[i];
		mi.m_prev_value = mi.m_value;
		mi.m_value = -1.0f;
	}

	// @TODO: locking!
	//g_sysMon->m_coreLoad.Update();

	// Go through every agent
	listnode * currentnode = nullptr;
	dolist(currentnode, g_sysMon->m_agenttype_systemmonitor_agents)
	{
		agent * const currentagent = (agent *) currentnode->value;

		agenttype_systemmonitor_details const * const details = static_cast<agenttype_systemmonitor_details const *>(currentagent->agentdetails);
		size_t const monitor_index = details->monitor_index;

		agenttype_systemmonitor_updatevalue(details);

		if (g_monitors[monitor_index].m_prev_value != g_monitors[monitor_index].m_value)
		{
			control_notify(currentagent->controlptr, NOTIFY_NEEDUPDATE, NULL);
		}
	}
}

void update_cpu_usage (agenttype_systemmonitor_details const * details)
{
	double foundvalue = 0.0;
	size_t const monitor_index = details->monitor_index;
	if (is2kxp)
	{
		//Variables
		long double diff_system_time;
		long double diff_idle_time;
		BOOL status;
		SYSTEM_TIME_INFORMATION SysTimeInfo;
		SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;

		//Get metrics
		status = g_sysMon->m_agenttype_systemmonitor_ntquerysysteminformation(agenttype_systemmonitor_SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
		if (status == NO_ERROR) status = g_sysMon->m_agenttype_systemmonitor_ntquerysysteminformation(agenttype_systemmonitor_SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);

		//If we succeeded
		if (status == NO_ERROR)
		{
			//Convert to long doubles
			long double current_system_time = (long double) SysTimeInfo.liKeSystemTime.QuadPart;
			long double current_idle_time = (long double) SysPerfInfo.liIdleTime.QuadPart;

			//If this is the first time
			if (g_sysMon->m_agenttype_systemmonitor_last_system_time == 0)
			{
				foundvalue = 0.0;
			}
			else
			{
				//Calculate difference
				diff_system_time = current_system_time - g_sysMon->m_agenttype_systemmonitor_last_system_time;
				diff_idle_time = current_idle_time - g_sysMon->m_agenttype_systemmonitor_last_idle_time;

				//Calculate usage
				foundvalue = 1.0 - (((diff_idle_time / diff_system_time)) / g_sysMon->m_agenttype_systemmonitor_number_processors);
				if (foundvalue < 0.0) foundvalue = 0.0;
				if (foundvalue > 1.0) foundvalue = 1.0;
			}

			//Set variables for next time
			g_sysMon->m_agenttype_systemmonitor_last_system_time = current_system_time;
			g_sysMon->m_agenttype_systemmonitor_last_idle_time = current_idle_time;

			//Set what we got
			g_monitors[monitor_index].m_value = foundvalue;
		}
	}
	else
	{
		HKEY hkey;
		DWORD dwDataSize;
		DWORD dwType;
		DWORD dwCpuUsage;

		RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StatData", 0, KEY_QUERY_VALUE, &hkey);
		dwDataSize = sizeof(dwCpuUsage);
		RegQueryValueEx(hkey, "KERNEL\\CPUUsage", NULL, &dwType, (LPBYTE)&dwCpuUsage, &dwDataSize);
		RegCloseKey(hkey);

		g_monitors[monitor_index].m_value = ((double)dwCpuUsage) / 100;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_systemmonitor_getvalue
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_systemmonitor_updatevalue (agenttype_systemmonitor_details const * details)
{
	size_t const monitor_index = details->monitor_index;
	//If we already know its value, then return it
	if (g_monitors[monitor_index].m_value != -1) return; // @TODO: epsilon

	//Get the memory info
	MEMORYSTATUS memstatus;
	GlobalMemoryStatus(&memstatus);

	switch (details->monitor_type)
	{
		case SYSTEMMONITOR_TYPE_CPUUSAGE:
			update_cpu_usage(details);
			break;
		case SYSTEMMONITOR_TYPE_PHYSMEMFREE:
			g_monitors[monitor_index].m_value = (100.0 - memstatus.dwMemoryLoad) / 100.0;
			break;
		case SYSTEMMONITOR_TYPE_PHYSMEMUSED:
			g_monitors[monitor_index].m_value = memstatus.dwMemoryLoad / 100.0;
			break;
		case SYSTEMMONITOR_TYPE_VIRTMEMFREE:
			g_monitors[monitor_index].m_value = memstatus.dwAvailVirtual / (double)memstatus.dwTotalVirtual;
			break;
		case SYSTEMMONITOR_TYPE_VIRTMEMUSED:
			g_monitors[monitor_index].m_value = 1.0 - (memstatus.dwAvailVirtual / (double)memstatus.dwTotalVirtual);
			break;
		case SYSTEMMONITOR_TYPE_PAGEFILEFREE:
			g_monitors[monitor_index].m_value = memstatus.dwAvailPageFile / (double)memstatus.dwTotalPageFile;
			break;
		case SYSTEMMONITOR_TYPE_PAGEFILEUSED:
			g_monitors[monitor_index].m_value = 1.0 - (memstatus.dwAvailPageFile / (double)memstatus.dwTotalPageFile);
			break;

		case SYSTEMMONITOR_TYPE_BATTERYPOWER:
		{
			SYSTEM_POWER_STATUS powerstatus;
			GetSystemPowerStatus(&powerstatus);
			double foundvalue = powerstatus.BatteryLifePercent / 100.0;
			if (foundvalue < 0.0) foundvalue = 0.0;
			else if (foundvalue > 1.0) foundvalue = 1.0;
			g_monitors[monitor_index].m_value = foundvalue;
			break;
		}

		case SYSTEMMONITOR_TYPE_COREUSAGE:
		{
			if (g_sysMon->m_coreLoad.HasCoreInfo(details->m_int_arg))
				g_monitors[monitor_index].m_value = g_sysMon->m_coreLoad.GetCoreInfo(details->m_int_arg).m_proc / 100.0;
			break;
		}
		default:
			g_monitors[monitor_index].m_value = 0.0; // Should never happen
			break;
	}

	int const intvalue = static_cast<int>(100.0 * g_monitors[monitor_index].m_value);
	g_monitors[monitor_index].m_str_value = std::to_string(intvalue) + "%";
	return;
}

