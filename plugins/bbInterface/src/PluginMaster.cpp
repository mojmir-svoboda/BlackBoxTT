/*===================================================

	PLUGIN MASTER CODE

===================================================*/
// Global Include
#include <blackbox/plugin/bb.h>
#include <blackbox/BlackBox_compat.h>
#include <bblib/utils_paths.h>

//Define the ALPHA SOFTWARE flag
//This will cause an annoying message box to pop up and confirm
//that the user wants to load it.  Comment in or out as desired.
//#define BBINTERFACE_ALPHA_SOFTWARE

#include <comutil.h>
#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#endif
#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <map>

//Parent Include
#include "PluginMaster.h"

//Includes
#include "Definitions.h"
#include "StyleMaster.h"
#include "WindowMaster.h"
#include "ControlMaster.h"
#include "AgentMaster.h"
#include "MessageMaster.h"
#include "ConfigMaster.h"
#include "DialogMaster.h"
#include "MenuMaster.h"
#include "Tooltip.h"
#include "ModuleMaster.h"

#include "ControlType_Label.h"
#include "ControlType_Button.h"
#include "ControlType_Slider.h"

#include "AgentType_Broam.h"
#include "AgentType_Winamp.h"
//#include "AgentType_iTunes.h"
#include "AgentType_Mixer.h"
#include "AgentType_StaticText.h"
#include "AgentType_CompoundText.h"
#include "AgentType_Run.h"
#include "AgentType_Bitmap.h"
#include "AgentType_TGA.h"
#include "AgentType_System.h"
#include "AgentType_SystemMonitor.h"
#include "AgentType_DiskSpaceMonitor.h"
#include "AgentType_NetworkMonitor.h"
#include "AgentType_SwitchedState.h"
#include "AgentType_Graph.h"
#include "AgentType_AutoScaleGraph.h"
#include "AgentType_SystemInfo.h"
#include "AgentType_Clock.h"

#ifdef _DEBUG
//Memory leak tracking - not used in production version
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

/*=================================================*/
//Plugin information
const char szVersionA[] = "BBInterface 0.9.10_bbTT";   // Used in MessageBox titlebars
const char szAppNameA[] = "BBInterface";          // The name of our window class, etc.
const char szInfoVersionA[] = "0.9.10_bbTT";
const char szInfoAuthorA[] = "psyci - additions by grischka/kana/ysuke/pkt-zer0/kazmix";
const char szInfoRelDateA[] = "2016/10/09";
const char szInfoLinkA[] = "";
const char szInfoEmailA[] = "";

const wchar_t szVersion      [] = L"BBInterface 0.9.10_bbTT";   // Used in MessageBox titlebars
const wchar_t szAppName      [] = L"BBInterface";          // The name of our window class, etc.
const wchar_t szInfoVersion  [] = L"0.9.10_bbTT";
const wchar_t szInfoAuthor   [] = L"psyci - additions by grischka/kana/ysuke/pkt-zer0/kazmix";
const wchar_t szInfoRelDate  [] = L"2010/10/09";
const wchar_t szInfoLink     [] = L"";
const wchar_t szInfoEmail    [] = L"";

//Local variables
const wchar_t szPluginAbout  [] =
L"BBInterface - Written Feb 2004 by psyci in many long, caffeine filled hours."
L"\n"
L"\nThanks to:"
L"\n    The BlackBox for Windows Development Team,"
L"\n    the folks from Freenode #bb4win,"
L"\n    \"jglatt\", for loads of information on Windows Mixers,"
L"\n    and a very sexy young lady who somehow put up with me coding until 5 AM\t"
L"\n    while she tried to sleep five feet away."
L"\n"
L"\nBBInterface is licensed under the GPL."
L"\n"
L"\nHistory:"
L"\nFeb 2004 - Original release by psyci."
L"\nNov 2004 - Snap controls and plugin management added by grischka."
L"\nMay 2006 - Added new agents and window properties by psyci."
;

const wchar_t szPluginAboutLastControl [] =
L"This control has been created because there are no more\n"
L"BBInterface controls.\n\n"
L"To create a new one, Control+Right Click this button, and\n"
L"use the menus to create new controls.\n\n"
L"If you don't want any more controls, please just unload\n"
L"the plugin."
;

const wchar_t szPluginAboutQuickRef [] =
L"Control + Right Click = Show Menu.\n"
L"Control + Drag = Move\n"
L"Control + Shift + Drag = Move Parent\n"
L"Alt + Drag = Resize\n"
;

//Strings used frequently
const wchar_t szBBroam                     []  = L"@BBInterface";
const int szBBroamLength = sizeof szBBroam-1;

const wchar_t szBEntityControl             [] = L"Control";
const wchar_t szBEntityAgent               [] = L"Agent";
const wchar_t szBEntityPlugin              [] = L"Plugin";
const wchar_t szBEntityVarset              [] = L"Set";
const wchar_t szBEntityWindow              [] = L"Window";
const wchar_t szBEntityModule              [] = L"Module";

const wchar_t szBActionCreate              [] = L"Create";
const wchar_t szBActionCreateChild         [] = L"CreateChild";
const wchar_t szBActionDelete              [] = L"Delete";

const wchar_t szBActionSetAgent            [] = L"SetAgent";
const wchar_t szBActionRemoveAgent         [] = L"RemoveAgent";

const wchar_t szBActionSetAgentProperty    [] = L"SetAgentProperty";
const wchar_t szBActionSetControlProperty  [] = L"SetControlProperty";  
const wchar_t szBActionSetWindowProperty   [] = L"SetWindowProperty";
const wchar_t szBActionSetPluginProperty   [] = L"SetPluginProperty";
const wchar_t szBActionSetModuleProperty   [] = L"SetModuleProperty";

//const wchar_t szBActionAddModule           [] = "AddModule";
const wchar_t szBActionToggle				[] = L"Toggle";
const wchar_t szBActionSetDefault		    [] = L"SetDefault";
const wchar_t szBActionAssignToModule      [] = L"AssignToModule";
const wchar_t szBActionDetachFromModule    [] = L"DetachFromModule";

const wchar_t szBActionOnLoad				[] = L"OnLoad";
const wchar_t szBActionOnUnload			[] = L"OnUnload";

const wchar_t szBActionRename              [] = L"Rename"; 
const wchar_t szBActionLoad                [] = L"Load";
const wchar_t szBActionEdit                [] = L"Edit";
const wchar_t szBActionSave                [] = L"Save";
const wchar_t szBActionSaveAs              [] = L"SaveAs"; 
const wchar_t szBActionRevert              [] = L"Revert"; 
const wchar_t szBActionAbout               [] = L"About";

const wchar_t szTrue                       [] = L"true";
const wchar_t szFalse                      [] = L"false";

const wchar_t szFilterProgram  [] = L"Programs\0*.exe;*.bat;*.com\0Screen Savers\0*.scr\0All Files\0*.*\0";
const wchar_t szFilterScript   [] = L"Script Files\0*.rc;\0All Files\0*.*\0";
const wchar_t szFilterAll      [] = L"All Files\0*.*\0";

//Convenient arrays of strings
const wchar_t *szBoolArray[2] = {  szFalse, szTrue };

/*=================================================*/
//Special Functions

extern "C"
{
	BBI_API int beginPlugin(HINSTANCE hMainInstance);
	BBI_API void endPlugin(HINSTANCE hMainInstance);
	BBI_API LPCSTR pluginInfo(int field);
	BBI_API int beginSlitPlugin(HINSTANCE hMainInstance, HWND hSlit);
	BBI_API int beginPluginEx(HINSTANCE hMainInstance, HWND hSlit);
	BBI_API LPCWSTR pluginInfoW(int field);
}

/*=================================================*/
//Global variables

HINSTANCE plugin_instance_plugin = NULL;
HWND plugin_hwnd_blackbox = NULL;
HWND plugin_hwnd_slit = NULL;

/*=================================================*/
// plugin configuration

bool plugin_using_modern_os = false;
bool plugin_zerocontrolsallowed = false;    
bool plugin_suppresserrors = true;
//bool plugin_enableshadows = false;
bool plugin_click_raise = false;
bool plugin_snapwindows = true;
// bool plugin_backup_script = true; // A global variable didn't seem to be the most compatible option.
wchar_t *plugin_desktop_drop_command = NULL;
bool plugin_clear_configuration_on_load = false;
bool plugin_save_modules_on_unload = true;

int plugin_snap_padding     = 2;
int plugin_snap_dist        = 7;
bool plugin_load = false;

int plugin_icon_sat = 255;
int plugin_icon_hue = 0;

//using COM 
bool com_initialized = false;
CComModule _Module;

//Font List
std::list<bbstring> fontList;
bool flist_initialized = false;
_locale_t locale = NULL;

//detect fullscreen app variables
bool fullscreen_app_exist = false;
HMONITOR fullscreen_app_hMon = NULL;

#define M_BOL 1
#define M_INT 2
#define M_STR 3

struct plugin_properties
{
	const wchar_t *key;
	const wchar_t *menutext;
	void *data;
	unsigned char type:3;
	unsigned char update:1;
	short minval;
	short maxval;
} plugin_properties[] =
{
	{ L"SnapWindows",            L"Snap To Edges",        &plugin_snapwindows,    M_BOL, 0 },
	{ L"SnapDistance",           L"Snap Trigger Distance", &plugin_snap_dist,     M_INT, 0,   0,  20 },
	{ L"SnapPadding",            L"Snap Padding",         &plugin_snap_padding,   M_INT, 0, -10,  40 },
	{ L"" },
	{ L"IconSaturation",         L"Icon Saturation",      &plugin_icon_sat,       M_INT, 1, 0, 255 },
	{ L"IconHue",                L"Icon Hue",             &plugin_icon_hue,       M_INT, 1, 0, 255 },
	{ L"" },
	{ L"DeskDropCommand",        L"Desktop OnDrop Command", &plugin_desktop_drop_command, M_STR, 0 },
	{ L"GlobalOnload",        L"Global OnLoad", &globalmodule.actions[MODULE_ACTION_ONLOAD], M_STR, 0 },
	{ L"GlobalOnunload",        L"Global OnUnload", &globalmodule.actions[MODULE_ACTION_ONUNLOAD], M_STR, 0 },
	{ L"" },
	{ L"ClickRaise",             L"Click Raise",          &plugin_click_raise,    M_BOL, 0 },
	{ L"SuppressErrors",         L"Suppress Errors",      &plugin_suppresserrors, M_BOL, 0 },
	{ L"ZeroControlsAllowed",    L"Allow Zero Controls",  &plugin_zerocontrolsallowed, M_BOL, 0 },
//	{ "EnableShadows",			"Enable Shadows",		&plugin_enableshadows, M_BOL, 1 },
	{ L"UseTooltip",             L"Use Tooltip",          &tooltip_enabled, M_BOL, 0 },
	{ L"ClearConfigurationOnLoad", L"Clear Configuration On Load", &plugin_clear_configuration_on_load, M_BOL, 0 },
	{ L"ModuleAutoSave", L"Save Modules On Unload", &plugin_save_modules_on_unload, M_BOL, 0 },
	{ NULL, NULL, 0 }
};

/*=================================================*/
//Internal functions
void plugin_getosinfo();
void plugin_controls_startup();
void plugin_controls_shutdown();
void plugin_agents_startup();
void plugin_agents_shutdown();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//beginPlugin
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int beginPlugin(HINSTANCE hMainInstance)
{
	if (plugin_hwnd_blackbox)
	{
		MessageBox(plugin_hwnd_blackbox, L"Dont load me twice!", szAppName, MB_OK|MB_SETFOREGROUND);
		return 1;
	}

	//Deal with instances
	plugin_instance_plugin = hMainInstance;
	plugin_hwnd_blackbox = GetBBWnd();

	const wchar_t *bbv = GetBBVersion();

	//Deal with os info
	plugin_getosinfo();

#ifdef BBINTERFACE_ALPHA_SOFTWARE
	int result = MessageBox(plugin_hwnd_blackbox,
		"WARNING!\n\n"
		"This is ALPHA software! Use at your own risk!\n\n"
		"The authors are not responsible in the event that:\n - your configuration is lost,\n - your computer blows up,\n - you are hit by a truck, or\n - anything else at all.\n\n"
		"Do you wish to continue loading this ALPHA software?",
		"BBInterface ALPHA Warning",
		MB_ICONWARNING|MB_DEFBUTTON2|MB_YESNO);
	if (result != IDYES) return 0;
#endif
	if(!com_initialized){
		if(SUCCEEDED(::CoInitialize(NULL)) ){
			_Module.Init(NULL, ::GetModuleHandle(NULL),NULL);
			com_initialized = true;
		}else{
			MessageBox(0, L"Error initializing COM", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
			return 1;
		}
	}

	//get font list 
	if(!flist_initialized){
		flist_initialized = true;
		HDC hdc;
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = 0;
		lf.lfFaceName[0]=0;
		hdc = GetDC(plugin_hwnd_blackbox);
		EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROC)EnumFontFamExProc,NULL,0);
		ReleaseDC(plugin_hwnd_blackbox,hdc);
		fontList.sort();
	}

	
	
	//Startup
	plugin_load = true;

	
	plugin_startup();

	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//beginPlugin
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int beginSlitPlugin(HINSTANCE hMainInstance, HWND hSlit)
{
	plugin_hwnd_slit = hSlit;
	return beginPlugin(hMainInstance);
}

int beginPluginEx(HINSTANCE hMainInstance, HWND hSlit)
{
	return beginSlitPlugin(hMainInstance, hSlit);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//endPlugin
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void endPlugin(HINSTANCE hMainInstance)
{
	if (plugin_load) plugin_shutdown(true);
	if(locale){
		_free_locale(locale);
		locale = NULL;
	}
	if(com_initialized){
		_Module.Term();
		::CoUninitialize(); 
		com_initialized = false;
	}

#ifdef _DEBUG
	//Memory leak tracking - not used in production version
	_CrtDumpMemoryLeaks();
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//pluginInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPCSTR pluginInfo(int field)
{
	switch (field)
	{
		default:
		case 0: return szVersionA;
		case 1: return szAppNameA;
		case 2: return szInfoVersionA;
		case 3: return szInfoAuthorA;
		case 4: return szInfoRelDateA;
		case 5: return szInfoLinkA;
		case 6: return szInfoEmailA;
	}
}

LPCWSTR pluginInfoW (int field)
{
	switch (field)
	{
	default:
	case 0: return szVersion;
	case 1: return szAppName;
	case 2: return szInfoVersion;
	case 3: return szInfoAuthor;
	case 4: return szInfoRelDate;
	case 5: return szInfoLink;
	case 6: return szInfoEmail;
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//plugin_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void plugin_startup()
{
	//Startup the configuration system
	config_startup();

	//Startup style system
	style_startup();

	//Startup window system
	window_startup();

	menu_startup();

	//Startup controls and agents
	variables_startup();
	control_startup();
	agent_startup();

	//Startup message & dialog system
	message_startup();
	dialog_startup();

	//Startup control types
	plugin_controls_startup();

	//Startup agent types
	plugin_agents_startup();

	tooltip_startup();
	module_startup();

	//Load the config file
	if (0 == config_load(config_path_mainscript, &globalmodule))
	{
		check_mainscript_filetime();
		if (!check_mainscript_version())
			SetTimer(message_window, 1, 1000, NULL);
	}
	control_checklast();

	message_interpret(globalmodule.actions[MODULE_ACTION_ONLOAD].c_str(), false, &globalmodule);

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//plugin_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void plugin_shutdown(bool save)
{
	message_interpret(globalmodule.actions[MODULE_ACTION_ONUNLOAD].c_str(), false, &globalmodule);

	//Save config settings
	if (save) config_save(config_path_mainscript);

	//Shutdown the message & dialog system
	message_shutdown();
	dialog_shutdown();

	//Shutdown agents and controls
	variables_shutdown();
	module_shutdown();
	control_shutdown();
	agent_shutdown();   

	//Shutdown the windowing system
	window_shutdown();
	
	//Shutdown the style system.
	style_shutdown();

	//Shutdown control types
	plugin_controls_shutdown();

	//Shutdown agents
	plugin_agents_shutdown();

	tooltip_shutdown();

	//Shutdown the configuration system
	config_shutdown();

	menu_shutdown();

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//plugin_reconfigure
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void _restart_specific_systems(void)
{
	//Shutdown specific systems
	module_shutdown();
	variables_shutdown();
	control_shutdown();
	agent_shutdown();
	style_shutdown();
	plugin_controls_shutdown();
	plugin_agents_shutdown();

	//Restart agent and control masters
	style_startup();
	control_startup();
	agent_startup();
	plugin_controls_startup();
	plugin_agents_startup();
	module_startup();
	variables_startup();

}

void plugin_reconfigure(bool force)
{
	if (force || (bb::fileExists(config_path_mainscript) && check_mainscript_filetime()))
	{
		_restart_specific_systems();

		//Reload config file
		config_load(config_path_mainscript, &globalmodule);
		control_checklast();
	}
	else
	{
		style_shutdown();
		style_startup();
		control_invalidate();
	}
}


static int config_load2(wchar_t *filename, module* caller)
{
	if (plugin_clear_configuration_on_load)
		_restart_specific_systems();
	return config_load(filename, caller);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//plugin_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int plugin_message(int tokencount, wchar_t *tokens[], bool from_core, module* caller)
{
	wchar_t *filename;

	if (tokencount == 3 && !_wcsicmp(tokens[2], szBActionSave))
	{
		config_save(config_path_mainscript);
		return 0;
	}
	else if (!_wcsicmp(tokens[2], szBActionSaveAs))
	{
		if (tokencount == 4)
		{
			config_save(tokens[3]);
		}
		else
		{       
			if ((filename = dialog_file(szFilterScript, L"Save Configuration Script", L".rc", config_path_plugin, true)))
			{
				config_save(filename);
			}           
		}       
		return 0;
	}
	else if (tokencount == 3 && !_wcsicmp(tokens[2], szBActionRevert))
	{
		plugin_reconfigure(true);
		return 0;
	}
	else if (!_wcsicmp(tokens[2], szBActionLoad))
	{
		if (tokencount == 4)
		{
			config_load2(tokens[3], caller);
			return 0;
		}
		else if (tokencount == 3)
		{
			if ((filename = dialog_file(szFilterScript, L"Load Configuration Script", L".rc", config_path_plugin, false)))
			{
				config_load2(filename, caller);
			}
			return 0;
		}
		else if (tokencount == 6)
		{
			if (0 == _wcsicmp(tokens[4], L"from"))
			{
				config_load(tokens[5], caller, tokens[3]);
				return 0;
			} else if (0 == _wcsicmp(tokens[4], L"into"))
			{
				config_load(tokens[3], module_get(tokens[5]));
				return 0;
			}
		}
		else if (tokencount == 8 && 0 == _wcsicmp(tokens[4], L"from") && 0 == _wcsicmp(tokens[6], L"into"))
		{
			config_load(tokens[5], module_get(tokens[7]), tokens[3]);
			return 0;
		}
	}
	else if (!_wcsicmp(tokens[2], szBActionAbout))
	{
		if (tokencount == 3)
		{
			MessageBox(NULL, szPluginAbout, szVersion, MB_OK|MB_SYSTEMMODAL);
			return 0;
		}
		else if (tokencount == 4 && !_wcsicmp(tokens[3], L"LastControl"))
		{
			MessageBox(NULL, szPluginAboutLastControl, szAppName, MB_OK|MB_SYSTEMMODAL);
			return 0;
		}
		else if (tokencount == 4 && !_wcsicmp(tokens[3], L"QuickHelp"))
		{
			MessageBox(NULL, szPluginAboutQuickRef, szAppName, MB_OK|MB_SYSTEMMODAL);
			return 0;
		}
	}
	else if (!_wcsicmp(tokens[2], szBActionEdit))
	{
		//SendMessage(plugin_hwnd_blackbox, BB_EDITFILE, (WPARAM)-1, (LPARAM) config_path_mainscript);
		//return 0;
		wchar_t temp[MAX_PATH]; GetBlackboxEditor(temp, MAX_PATH);
		BBExecute(NULL, L"", temp , config_path_mainscript, NULL, SW_SHOWNORMAL, false);
		return 0;
	}
	else if (tokencount == 5 && !_wcsicmp(tokens[2], szBActionSetPluginProperty))
	{
		for (struct plugin_properties *p = plugin_properties; p->key; p++)
			if (p->data && 0 == _wcsicmp(tokens[3], p->key)) {
				switch (p->type) {
					case M_BOL:
						if (config_set_bool(tokens[4], (bool*)p->data)) break; return 1;
					case M_INT:
						if (config_set_int(tokens[4], (int*)p->data)) break; return 1;
					case M_STR:
						if (config_set_str(tokens[4], (wchar_t**)p->data)) break; return 1;
					default: return 1;
				}
				if (p->update) control_invalidate();
				if (from_core) menu_update_global();
				return 0;
			}
	}
	else if (tokencount == 4 && !_wcsicmp(tokens[2], szBActionOnLoad) )
	{
		config_set_str(tokens[3], globalmodule.actions[MODULE_ACTION_ONLOAD]);
		return 0;
	}
	else if (tokencount == 4 && !_wcsicmp(tokens[2], szBActionOnUnload) )
	{
		config_set_str(tokens[3], globalmodule.actions[MODULE_ACTION_ONUNLOAD]);
		return 0;
	}
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//plugin_save
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void plugin_save()
{
	config_printf(L"!---- %s ----", szVersion);
	for (struct plugin_properties *p = plugin_properties; p->key; p++)
		if (p->data) switch (p->type) {
			case M_BOL: config_write(config_get_plugin_setpluginprop_b(p->key, (bool*)p->data)); break;
			case M_INT: config_write(config_get_plugin_setpluginprop_i(p->key, (const int*)p->data)); break;
			case M_STR: config_write(config_get_plugin_setpluginprop_c(p->key, *(const wchar_t**)p->data)); break;
		}
	//Save OnLoad/OnUnload actions
	config_write(config_get_plugin_onload());
	config_write(config_get_plugin_onunload());

	variables_save();

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
std::shared_ptr<bb::MenuConfig> plugin_menu_settings(void)
{
	std::shared_ptr<bb::MenuConfig> m = make_menu(L"Global Options");
	bool tmp;
	for (struct plugin_properties *p = plugin_properties; p->key; p++)
		switch (p->type)
		{
			case M_BOL:
				tmp = false == *(bool*)p->data;
				make_menuitem_bol(m, p->menutext, config_get_plugin_setpluginprop_b(p->key, &tmp), !tmp);
				break;
			case M_INT:
				make_menuitem_int(m, p->menutext, config_get_plugin_setpluginprop_s(p->key), *(const int*)p->data, p->minval, p->maxval);
				break;
			case M_STR:
				make_menuitem_str(m, p->menutext, config_get_plugin_setpluginprop_s(p->key), *(const wchar_t **)p->data ? *(const wchar_t**)p->data : L"");
				break;
			default:
				make_menuitem_nop(m, NULL);
				break;
		}
	return m;
}

//##################################################
//plugin_getosinfo
//##################################################
void plugin_getosinfo()
{
	//This code stolen from the bb4win SDK example
	OSVERSIONINFO osvinfo;
	ZeroMemory(&osvinfo, sizeof(osvinfo));
	osvinfo.dwOSVersionInfoSize = sizeof (osvinfo);
	GetVersionEx(&osvinfo);
	plugin_using_modern_os =
		osvinfo.dwPlatformId == VER_PLATFORM_WIN32_NT
		&& osvinfo.dwMajorVersion > 4;
}

//##################################################
//plugin_controls_startup
//##################################################
void plugin_controls_startup()
{
	controltype_label_startup();
	controltype_button_startup();
	controltype_slider_startup();
}

//##################################################
//plugin_controls_shutdown
//##################################################
void plugin_controls_shutdown()
{
	controltype_label_shutdown();
	controltype_button_shutdown();
	controltype_slider_shutdown();
}

//##################################################
//plugin_agents_startup
//##################################################
void plugin_agents_startup()
{
	agenttype_run_startup();
	agenttype_broam_startup();
	agenttype_system_startup();
	agenttype_winamp_startup();
//	agenttype_itunes_startup();
	agenttype_mixer_startup();
	agenttype_statictext_startup();
	agenttype_compoundtext_startup();
	agenttype_bitmap_startup();
	agenttype_tga_startup();
	agenttype_systemmonitor_startup();
	agenttype_diskspacemonitor_startup();
	agenttype_networkmonitor_startup();
	agenttype_systeminfo_startup();
	agenttype_clock_startup();
	agenttype_switchedstate_startup();
	agenttype_graph_startup();
	agenttype_autoscalegraph_startup();
}

//##################################################
//plugin_agents_shutdown
//##################################################
void plugin_agents_shutdown()
{
	agenttype_run_shutdown();
	agenttype_broam_shutdown();
	agenttype_system_shutdown();
	agenttype_winamp_shutdown();
//	agenttype_itunes_shutdown();
	agenttype_mixer_shutdown();
	agenttype_statictext_shutdown();
	agenttype_compoundtext_shutdown();
	agenttype_bitmap_shutdown();
	agenttype_tga_shutdown();
	agenttype_systemmonitor_shutdown();
	agenttype_diskspacemonitor_shutdown();
	agenttype_networkmonitor_shutdown();
	agenttype_systeminfo_shutdown();
	agenttype_clock_shutdown();
	agenttype_switchedstate_shutdown();
	agenttype_graph_shutdown();
	agenttype_autoscalegraph_shutdown();
}

//
//



int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,int FontType , LPARAM lParam) {
	if(FontType != TRUETYPE_FONTTYPE)
     return 1;

	static std::map<bbstring, int> unique_check;

	wchar_t *tmp = (wchar_t *)lpelfe->elfLogFont.lfFaceName;
	if(*tmp != L'@' && unique_check[tmp] == 0){
		unique_check[tmp]=1;
		fontList.push_back(lpelfe->elfLogFont.lfFaceName);
	}
	return 1;
}
bool detect_fullscreen_window(){
	bool oldvalue = fullscreen_app_exist;
	fullscreen_app_exist = search_fullscreen_app();
	return oldvalue != fullscreen_app_exist;
}

bool search_fullscreen_app(){	
	HWND fg_hwnd = GetForegroundWindow();
	LONG_PTR style = GetWindowLongPtr(fg_hwnd, GWL_STYLE);
	if (WS_CAPTION == (style & WS_CAPTION))
		return false;
	RECT s;
	fullscreen_app_hMon = GetMonitorRect(fg_hwnd, &s, GETMON_FROM_WINDOW);
	RECT r;
	GetWindowRect(fg_hwnd, &r);
	return r.right - r.left >= s.right - s.left && r.bottom - r.top >= s.bottom - s.top;
}
