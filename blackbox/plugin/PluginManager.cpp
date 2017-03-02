/* ==========================================================================
This file is part of the bbLean source code
Copyright © 2001-2003 The Blackbox for Windows Development Team
Copyright © 2004-2009 grischka
Copyright © 201.-201? 

http://bb4win.sourceforge.net/bblean
http://developer.berlios.de/projects/bblean

bbLean is free software, released under the GNU General Public License
(GPL version 2). For details see:
========================================================================== */
#include "PluginManager.h"
#include <regex>
#include <string.h>
#include "PluginLoaderNative.h"
#include "Types.h"
#include "PluginsConfig.h"
#include <blackbox/common.h>

namespace bb {

void PluginManager::LoadPlugin (bbstring const & plugin_id)
{
	for (PluginInfoPtr & pi : m_infos.m_infos)
	{
		if (plugin_id == pi->m_config.m_name && !pi->m_enabled)
		{
			TRACE_MSG(LL_INFO, CTX_BB | CTX_PLUGINMGR, "Loading plugin: %ws", pi->m_config.m_name.c_str());
			plugin_errors const ret = pi->LoadPlugin(m_hSlit);
			if (ret == error_plugin_success)
			{
				if (pi->m_config.m_isSlit)
				{
					m_hSlit = FindWindow(L"bbSlit", NULL); // OMFG 2
				}
				TRACE_MSG(LL_DEBUG, CTX_BB | CTX_PLUGINMGR, "ok, slit hwnd=0x%x", m_hSlit);
			}
			else
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_PLUGINMGR, "error, code=0x%x", ret);
			}
		}
	}
}
void PluginManager::UnloadPlugin (bbstring const & plugin_id)
{
	for (PluginInfoPtr & pi : m_infos.m_infos)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_PLUGINMGR, "Unloading plugin: %ws", pi->m_config.m_name.c_str());
		if (plugin_id == pi->m_config.m_name && pi->m_enabled)
		{
			pi->UnloadPlugin();
			if (pi->m_isSlit)
				m_hSlit = nullptr;
		}
	}
}
bool PluginManager::IsPluginLoaded (bbstring const & plugin_id) const
{
	for (PluginInfoPtr const & pi : m_infos.m_infos)
		if (plugin_id == pi->m_config.m_name && pi->m_enabled)
			return true;
	return false;
}

bool PluginManager::Init (PluginsConfig const & cfg)
{
	TRACE_SCOPE(LL_INFO, CTX_BB | CTX_PLUGINMGR | CTX_INIT);

	std::vector<PluginInfoPtr> m_tmp = std::move(m_infos.m_infos);

	for (PluginConfig const & pc : cfg.m_plugins)
	{
		bool found = false;
		for (PluginInfoPtr & pi : m_tmp)
		{
			if (pi->m_config.m_path == pc.m_path /*&& pi->m_config.m_instance == pc.m_instance*/)
			{
				m_infos.m_infos.push_back(std::move(pi));
				found = true;
				break;
			}
		}

		if (!found)
		{
			PluginInfoPtr pi(new PluginInfo);
			pi->InitFromConfig(pc);
			pi->m_enabled = false;
			m_infos.m_infos.push_back(std::move(pi));
		}
	}

	// @TODO: find slit, run slit first, then load the rest and unload slit last
	for (PluginInfoPtr & pi : m_infos.m_infos)
	{
		if (pi->m_config.m_isSlit && pi->m_module == nullptr && pi->m_config.m_enabled)
		{
			TRACE_MSG(LL_INFO, CTX_BB | CTX_PLUGINMGR, "Loading slit plugin: %ws", pi->m_config.m_name.c_str());
			plugin_errors const ret = pi->LoadPlugin(nullptr);
			if (ret == error_plugin_success)
			{
				m_hSlit = FindWindow(L"bbSlit", NULL); // OMFG
				TRACE_MSG(LL_DEBUG, CTX_BB | CTX_PLUGINMGR, "ok, slit hwnd=0x%x", m_hSlit);
			}
			else
			{
				TRACE_MSG(LL_DEBUG, CTX_BB | CTX_PLUGINMGR, "error, code=0x%x", ret);
			}
		}
	}

	for (PluginInfoPtr & pi : m_infos.m_infos)
	{
		if (pi->m_module == nullptr && pi->m_config.m_enabled)
		{
			TRACE_MSG(LL_INFO, CTX_BB | CTX_PLUGINMGR, "Loading plugin: %ws", pi->m_config.m_name.c_str());
			plugin_errors const ret = pi->LoadPlugin(m_hSlit);
			if (ret == error_plugin_success)
			{
			}
		}
	}

	for (PluginInfoPtr & pi : m_tmp)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_PLUGINMGR, "Unloading plugin: %ws", pi->m_config.m_name.c_str());
		pi->UnloadPlugin();
	}
	m_tmp.clear();
	return true;
}

bool PluginManager::Done ()
{
	TRACE_SCOPE_MSG(LL_INFO, CTX_BB | CTX_PLUGINMGR, "Terminating plugin manager");
	return true;
}

template <class T>
bool LoadFunction (HINSTANCE mod, char const * name, T & fn)
{
	FARPROC proc = GetProcAddress(mod, name);
	fn = reinterpret_cast<T>(proc);
	return fn != nullptr;
}

plugin_errors PluginInfo::LoadPlugin (HWND hSlit)
{
	plugin_errors error = error_plugin_success;
	bool useslit = false;
	int r = 0;
	wchar_t plugin_path[MAX_PATH];

	r = SetErrorMode(0); // enable 'missing xxx.dll' system message
	m_module = LoadLibrary(m_config.m_path.c_str());
	SetErrorMode(r);

	if (NULL == m_module)
	{
		r = GetLastError();
		// char buff[200]; win_error(buff, sizeof buff);
		// dbg_printf("LoadLibrary::GetLastError %d: %s", r, buff);
		if (ERROR_MOD_NOT_FOUND == r)
			return error_plugin_dll_needs_module;
		else
			return error_plugin_does_not_load;
	}

	//---------------------------------------
	// grab interface functions
	LoadFunction(m_module, c_beginPluginName, m_beginPlugin);
	LoadFunction(m_module, c_beginPluginExName, m_beginPluginEx);
	LoadFunction(m_module, c_beginSlitPluginName, m_beginSlitPlugin);
	LoadFunction(m_module, c_endPluginName, m_endPlugin);
	LoadFunction(m_module, c_pluginInfoName, m_pluginInfo);
	LoadFunction(m_module, c_pluginInfoWName, m_pluginInfoW);

	//---------------------------------------
	// check interfaces
	if (!m_endPlugin)
	{
		UnloadPlugin();
		return error_plugin_missing_entry;
	}

	m_isWideChar = m_pluginInfoW != nullptr;

	// check whether plugin supports the slit
	m_canUseSlit = !!m_beginPluginEx || !!m_beginSlitPlugin;

	if (!m_canUseSlit)
		m_inSlit = false;

	useslit = hSlit && m_inSlit;

	if (!useslit && !m_beginPluginEx && !m_beginPlugin)
	{
		UnloadPlugin();
		return error_plugin_missing_entry;
	}

	//---------------------------------------
	// inititalize plugin
	__try
	{
		if (useslit)
		{
			if (m_beginPluginEx)
				r = m_beginPluginEx(m_module, hSlit);
			else
				r = m_beginSlitPlugin(m_module, hSlit);
		}
		else
		{
			if (m_beginPlugin)
				r = m_beginPlugin(m_module);
			else
				r = m_beginPluginEx(m_module, NULL);
		}

		if (BEGINPLUGIN_OK == r)
		{
			m_inSlit = useslit;
		}
		else if (BEGINPLUGIN_FAILED_QUIET != r)
		{
			error = error_plugin_fail_to_load;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		error = error_plugin_crash_on_load;
	}

	// clean up after error
	if (error)
	{
		m_enabled = false;
		UnloadPlugin();
	}
	else
	{
		m_enabled = true;
	}
	return error;
}

bool PluginInfo::UnloadPlugin()
{
	plugin_errors error = error_plugin_success;
	__try
	{
		m_endPlugin(m_module);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		error = error_plugin_crash_on_unload;
	}

	if (m_module)
		FreeLibrary(m_module);
	m_enabled = false;
	m_canUseSlit = false;
	m_isSlit = false;
	m_inSlit = false;
	m_beginPlugin = nullptr;
	m_beginPluginEx = nullptr;
	m_beginSlitPlugin = nullptr;
	m_endPlugin = nullptr;
	m_pluginInfo = nullptr;
	return true;
}

//===========================================================================
// (API:) EnumPlugins
void EnumPlugins (PLUGINENUMPROC lpEnumFunc, LPARAM lParam)
{
}

//===========================================================================
// run through plugin list and load/unload changed plugins
void applyPluginState (struct PluginList *q)
{
//     char * errorMsg = "(Unknown error)";
// 
//     int error = 0;
//     if (q->loaderInfo) {
//         if (q->isEnabled && (q->useSlit && hSlit) == q->inSlit)
//             return;
//         error = unloadPlugin(q, &errorMsg);
//     }
// 
//     if (q->isEnabled) {
//         if (0 == error)
//             error = loadPlugin(q, hSlit, &errorMsg);
//         if (!q->loaderInfo)
//             q->isEnabled = false;
//         if (error)
//             write_plugins();
//     }
// 
//     showPluginErrorMessage(q, error, errorMsg);
}

void applyPluginStates ()
{
//     struct PluginList *q;
// 
//     hSlit = NULL;
// 
//     // load slit first
//     if(slitPlugin && slitPlugin->isEnabled) {
//         applyPluginState(slitPlugin);
//         hSlit = FindWindow("BBSlit", NULL);
//     }
// 
//     // load/unload other plugins
//     dolist(q, bbplugins)
//         if (q->name && q != slitPlugin)
//             applyPluginState(q);
// 
//     // unload slit last
//     if(slitPlugin && !slitPlugin->isEnabled)
//         applyPluginState(slitPlugin);
}

//===========================================================================
// plugin error message

static void showPluginErrorMessage(struct PluginList *q, int error, const char* msg)
{
//     switch (error)
//     {
//     case 0:
//         return;
//     case error_plugin_is_built_in      :
//         msg = NLS2("$Error_Plugin_IsBuiltIn$",
//             "Dont load this plugin with "BBAPPNAME". It is built-in."
//             ); break;
//     case error_plugin_dll_not_found    :
//         msg = NLS2("$Error_Plugin_NotFound$",
//             "The plugin was not found."
//             ); break;
//     case error_plugin_dll_needs_module :
//         msg = NLS2("$Error_Plugin_MissingModule$",
//             "The plugin cannot be loaded. Possible reason:"
//             "\n- The plugin requires another dll that is not there."
//             ); break;
//     case error_plugin_does_not_load    :
//         msg = NLS2("$Error_Plugin_DoesNotLoad$",
//             "The plugin cannot be loaded. Possible reasons:"
//             "\n- The plugin requires another dll that is not there."
//             "\n- The plugin is incompatible with the windows version."
//             "\n- The plugin is incompatible with this blackbox version."
//             ); break;
//     case error_plugin_missing_entry    :
//         msg = NLS2("$Error_Plugin_MissingEntry$",
//             "The plugin misses the begin- and/or endPlugin entry point. Possible reasons:"
//             "\n- The dll is not a plugin for Blackbox for Windows."
//             ); break;
//     case error_plugin_fail_to_load     :
//         msg = NLS2("$Error_Plugin_IniFailed$",
//             "The plugin could not be initialized."
//             ); break;
//     case error_plugin_crash_on_load    :
//         msg = NLS2("$Error_Plugin_CrashedOnLoad$",
//             "The plugin caused a general protection fault on initializing."
//             "\nPlease contact the plugin author."
//             ); break;
//     case error_plugin_crash_on_unload  :
//         msg = NLS2("$Error_Plugin_CrashedOnUnload$",
//             "The plugin caused a general protection fault on shutdown."
//             "\nPlease contact the plugin author."
//             ); break;
//         //default:
//         //    msg = "(Unknown Error)";
//     }
//     BBMessageBox(
//         MB_OK,
//         NLS2("$Error_Plugin$", "Error: %s\n%s"),
//         q->path,
//         msg
//         );
}

//===========================================================================
// load/unload one plugin

static int loadPlugin (struct PluginList *plugin, HWND hSlit, char** errorMsg)
{
//     if(plugin == nativeLoader.parent)
//         return 0;
//     
//     if(isPluginLoader(plugin->path))
//         return loadPluginLoader(plugin, errorMsg);
// 
//     struct PluginLoaderList *pll;
//     int lastError = 0;
// 
//     dolist(pll, pluginLoaders) {
//         lastError = pll->LoadPlugin(plugin, hSlit, errorMsg);
// 
//         if(lastError == 0) {
//             struct PluginPtr *pp = c_new<struct PluginPtr>();
//             pp->entry = plugin;
// 
//             append_node(&(pll->plugins), pp);
//             break;
//         }
//     }
// 
//     return lastError;
}

static int unloadPlugin (struct PluginList *q, char** errorMsg)
{
//     struct PluginLoaderList *pll;
//     struct PluginPtr* pp;
// 
//     dolist(pll, pluginLoaders)
//         if(q == pll->parent) {
//             if(pll == &nativeLoader)
//                 return 0;
// 
//             q->loaderInfo = NULL;
// 
//             return unloadPluginLoader(pll, errorMsg);
//         }
// 
//     dolist(pll, pluginLoaders) {
//         dolist(pp, pll->plugins) {
//             if(pp->entry == q) {
//                 int error = pll->UnloadPlugin(q, errorMsg);
//                 remove_item(&pll->plugins, pp);
//                 return error;
//             }
//         }
//     }

    return 0;
}

static void free_plugin(struct PluginList **pp)
{
//     PluginList *q = *pp;
//     PluginLoaderList *pll = 0;
// 
//     dolist(pll, pluginLoaders) {
//         if(pll->parent == q) {
//             if(pll == &nativeLoader)
//                 pll->parent = NULL;
//             else
//                 remove_item(&pluginLoaders, pll);
// 
//             break;
//         }
//     }
// 
//     free_str(&q->name);
//     free_str(&q->path);
//     *pp = q->m_next;
//     m_free(q);
}

//===========================================================================
// write back the plugin list to "plugins.rc"

static bool write_plugins(void)
{
    return true;
}

static bool isPluginLoader (char * path) 
{
//     char ppl_path[MAX_PATH];
//     if (0 == findRcFile(ppl_path, path, NULL))
//         return false;
// 
//     // HACK: this way of loading the dll is actually not recommended, but it is the cheapest way to work with the image
//     HMODULE pll = LoadLibraryEx(path, NULL, DONT_RESOLVE_DLL_REFERENCES);
// 
//     if(pll == NULL) {
//         return false;
//     }
// 
//     // walking the headers, checking consistency as we go
//     PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)pll;
// 
//     if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
//         FreeLibrary(pll);
//         return false;
//     }
// 
//     PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(dosHeader->e_lfanew + (SIZE_T)pll);
//     if(ntHeader->Signature != IMAGE_NT_SIGNATURE) {
//         FreeLibrary(pll);
//         return false;
//     }
// 
// #ifdef _WIN64
//     if(ntHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
// #else
//     if(ntHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
// #endif
//         FreeLibrary(pll);
//         return false;
//     }
// 
//     PIMAGE_OPTIONAL_HEADER optHeader = &ntHeader->OptionalHeader;
// 
//     if(optHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) {
//         FreeLibrary(pll);
//         return false;
//     }
// 
//     // checking whether the module exports the functions which a pluginLoader must define
//     DWORD va = optHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
// 
//     PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)(va + (SIZE_T)pll);
//     DWORD numberOfFunctions = exportDir->NumberOfNames;
//     DWORD* names = (DWORD*)(exportDir->AddressOfNames + (SIZE_T)pll);
// 
//     DWORD numLoaderFunctions = 0;
//     for(DWORD j = 0; pluginLoaderFunctionNames[j]; j++)
//         numLoaderFunctions++;
// 
//     DWORD exportsFound = 0;
// 
//     for(DWORD i = 0; i < numberOfFunctions; i++) {
//         char* exportName = (char*)(*names + (SIZE_T)pll);
// 
//         for(DWORD j=0; pluginLoaderFunctionNames[j]; j++) {
//             if(!strcmp(exportName, pluginLoaderFunctionNames[j])) {
//                 exportsFound++;
//                 break;
//             }
//         }
// 
//         if(exportsFound == numLoaderFunctions)
//             break;
// 
//         names++;
//     }
// 
//     FreeLibrary(pll);
// 
//     return exportsFound == numLoaderFunctions;
}

static int loadPluginLoader(struct PluginList* plugin, char** errorMsg)
{
//     tstring ppl_path;
//     if (0 == findRcFile(ppl_path, plugin->path, NULL))
//         return error_plugin_dll_not_found;
// 
//     UINT r = SetErrorMode(0); // enable 'missing xxx.dll' system message
//     HMODULE module = LoadLibrary(ppl_path);
//     SetErrorMode(r);
// 
//     if(module == NULL) {
//         r = GetLastError();
// 
//         if (ERROR_MOD_NOT_FOUND == r)
//             return error_plugin_dll_needs_module;
//         else
//             return error_plugin_does_not_load;
//     }
// 
//     struct PluginLoaderList* pll = c_new<struct PluginLoaderList>();
//     pll->module = module;
// 
//     r = initPluginLoader(pll, errorMsg);
// 
//     if (0 == r) {
//         plugin->useSlit = false;
//         plugin->loaderInfo = pll;
//         
//         //plugin->name = (char *)realloc(plugin->name, strlen(pll->name) + 1);
//         //strcpy(plugin->name, pll->name);
// 
//         pll->parent = plugin;
//       
//         append_node(&pluginLoaders, pll);
// 
//         return 0;
//     }
// 
//     FreeLibrary(module);
//     c_del(pll);
//     return r;
}

static int initPluginLoader(struct PluginLoaderList* pll, char** errorMsg)
{
//     if(!LoadFunction(pll, Init))
//         FailWithMsg(errorMsg, "init not present in pluginLoader");
//     
//     if(!LoadFunction(pll, Finalize))
//         FailWithMsg(errorMsg, "finalize not present in pluginLoader");
//     
//     if(!LoadFunction(pll, GetName))
//         FailWithMsg(errorMsg, "getName not present in pluginLoader");
// 
//     if(!LoadFunction(pll, GetApi))
//         FailWithMsg(errorMsg, "getApi not present in pluginLoader");
// 
//     if(!LoadFunction(pll, LoadPlugin))
//         FailWithMsg(errorMsg, "loadPlugin not present in pluginLoader");
// 
//     if(!LoadFunction(pll, UnloadPlugin))
//         FailWithMsg(errorMsg, "unloadPlugin not present in pluginLoader");
//     
//     char pluginPath[MAX_PATH];
//     char pluginWorkDir[MAX_PATH];
// 
//     GetModuleFileName(pll->module, pluginPath, MAX_PATH);
// 
//     file_directory(pluginWorkDir, pluginPath);
// 
//     if(!pll->Init(pluginWorkDir))
//         FailWithMsg(errorMsg, "PluginLoader could not initialize.");
// 
//     pll->name = pll->GetName();
//     pll->api = pll->GetApi();

    return 0;
}

static int unloadPluginLoader(struct PluginLoaderList *loader, char** errorMsg) {
//     if(!loader->module)
//         return 0;
// 
//     struct PluginPtr *pp;
// 
//     int lastError = 0;
// 
//     dolist(pp, loader->plugins) {
//         lastError = loader->UnloadPlugin(pp->entry, errorMsg);
//         remove_item(&loader->plugins, pp);
//     }
//     
//     loader->Finalize();
// 
//     FreeLibrary(loader->module);
//     loader->module = NULL;
// 
//     remove_node(&pluginLoaders, loader);
// 
//     return lastError;
}

//===========================================================================


void PluginManager::AboutPlugins ()
{
//     int l, x = 0;
//     char *msg = (char*)c_alloc(l = 4096);
//     const char* (*pluginInfo)(struct PluginList *q, int);
//     struct PluginPtr *pp;
//     struct PluginList *q;
//     struct PluginLoaderList* pll;
// 
//     dolist(pll, pluginLoaders) {
//         if (l - x < MAX_PATH + 100)
//             msg = (char*)m_realloc(msg, l*=2);
//         if (x)
//             msg[x++] = '\n';
//         x += sprintf(msg + x, "%s\t", pll->name);
//     }
//     
//     dolist(pll, pluginLoaders) {
//         dolist(pp, pll->plugins) {
//             q = pp->entry;
// 
//             if (q->loaderInfo) {
//                 if (l - x < MAX_PATH + 100)
//                     msg = (char*)m_realloc(msg, l*=2);
//                 if (x)
//                     msg[x++] = '\n';
//             
//                 pluginInfo = pll->GetPluginInfo;
//                 if (pluginInfo)
//                     x += sprintf(msg + x,
//                     "%s %s %s %s (%s)\t",
//                     pluginInfo(q, PLUGIN_NAME),
//                     pluginInfo(q, PLUGIN_VERSION),
//                     NLS2("$About_Plugins_By$", "by"),
//                     pluginInfo(q, PLUGIN_AUTHOR),
//                     pluginInfo(q, PLUGIN_RELEASE)
//                     );
//                 else
//                     x += sprintf(msg + x, "%s\t", q->name);
//             }
//         }
//     }
//     
//     BBMessageBox(MB_OK,
//         "#"BBAPPNAME" - %s#%s\t",
//         NLS2("$About_Plugins_Title$", "About loaded plugins"),
//         x ? msg : NLS1("No plugins loaded.")
//         );
// 
//     m_free(msg);
}

//===========================================================================
// The plugin configuration menu

/*Menu *get_menu (const char *title, char *menu_id, bool pop, struct PluginList **qp, bool b_slit)
{
    struct PluginList *q;
    char *end_id;
    MenuItem *pItem;
    char command[20], label[80], broam[MAX_PATH+80];
    const char *cp;
    Menu *pMenu, *pSub;

    end_id = strchr(menu_id, 0);
    pMenu = MakeNamedMenu(title, menu_id, pop);

    while (NULL != (q = *qp)) {
        *qp = q->m_next;
        if (q->name) {
            if (0 == b_slit) {
                sprintf(broam, "@BBCfg.plugin.load %s", q->name);
                pItem = MakeMenuItem(pMenu, q->name, broam, q->isEnabled);
            } else if(q->isEnabled && q->canUseSlit) {
                sprintf(broam, "@BBCfg.plugin.inslit %s", q->name);
                MakeMenuItem(pMenu, q->name, broam, q->useSlit);
            }
        } else if (0 == b_slit) {
            cp = q->path;
            if (false == get_string_within(command, sizeof command, &cp, "[]"))
                continue;
            get_string_within(label, sizeof label, &cp, "()");
            if (0 == _stricmp(command, "nop"))
                MakeMenuNOP(pMenu, label);
            else if (0 == _stricmp(command, "sep"))
                MakeMenuNOP(pMenu, NULL);
            else if (0 == _stricmp(command, "submenu") && *label) {
                sprintf(end_id, "_%s", label);
                pSub = get_menu(label, menu_id, pop, qp, b_slit);
                MakeSubmenu(pMenu, pSub, NULL);
            } else if (0 == _stricmp(command, "end")) {
                break;
            }
        }
    }
    return pMenu;
}

Menu* PluginManager_GetMenu(const char *text, char *menu_id, bool pop, int mode)
{
    struct PluginList *q = bbplugins->m_next;
    if (SUB_PLUGIN_SLIT == mode && NULL == hSlit)
        return NULL;
    return get_menu(text, menu_id, pop, &q, SUB_PLUGIN_SLIT == mode);
}

//===========================================================================
// parse a line from plugins.rc to obtain the pluginRC address
bool parse_pluginRC(const char *rcline, const char *name)
{
    bool is_comment = false;

    if ('#' == *rcline || 0 == *rcline)
        is_comment = true;
    else
        if ('!' == *rcline)
            while (' '== *++rcline);

    if ('&' == *rcline)
    {
        while (' '== *++rcline);
    }

    if (!is_comment && IsInString(rcline, name))
    {
        char editor[MAX_PATH];
        char road[MAX_PATH];
        char buffer[2*MAX_PATH];
        char *s = strcpy((char*)name, rcline);
        *(char*)file_extension(s) = 0; // strip ".dll"
        if (IsInString(s, "+"))
            *(char*)get_delim(s, '+') = 0; // strip "+"
        rcline = (const char*)strcat((char*)s, ".rc");
        GetBlackboxEditor(editor);
        sprintf(buffer, "\"%s\" \"%s\"", editor, set_my_path(hMainInstance, road, rcline));
        BBExecute_string(buffer, RUN_SHOWERRORS);
        return true;
    }
    return false;
}

//===========================================================================
// parse a line from plugins.rc to obtain the Documentation address
bool parse_pluginDocs(const char *rcline, const char *name)
{
    bool is_comment = false;

    if ('#' == *rcline || 0 == *rcline)
        is_comment = true;
    else
        if ('!' == *rcline)
            while (' '== *++rcline);

    if ('&' == *rcline)
    {
        while (' '== *++rcline);
    }

    if (!is_comment && IsInString(rcline, name))
    {
        char road[MAX_PATH];
        char *s = strcpy((char*)name, rcline);
        *(char*)file_extension(s) = 0; // strip ".dll"
        // files could be *.html, *.htm, *.txt, *.xml ...
        if (locate_file(hMainInstance, road, s, "html") || locate_file(hMainInstance, road, s, "htm") || locate_file(hMainInstance, road, s, "txt") || locate_file(hMainInstance, road, s, "xml"))
        {
            BBExecute(NULL, "open", road, NULL, NULL, SW_SHOWNORMAL, 		false);
            return true;
        }
        *(char*)get_delim(s, '\\') = 0; // strip plugin name
        rcline = (const char*)strcat((char*)s, "\\readme");
        // ... also the old standby: readme.txt
        if (locate_file(hMainInstance, road, rcline, "txt"))
        {
            BBExecute(NULL, "open", road, NULL, NULL, SW_SHOWNORMAL, 		false);
            return true;
        }
    }
    return false;
}
*/

int PluginManager::HandleBroam (char const * args)
{
	return 0;
//     static const char * const actions[] = {
//         "add", "remove", "load", "inslit", "edit", "docs", NULL
//     };
//     enum {
//         e_add, e_remove, e_load, e_inslit, e_edit, e_docs
//     };
// 
//     char buffer[MAX_PATH];
//     struct PluginList *q;
//     int action;
// 
//     NextToken(buffer, &args, NULL);
//     action = get_string_index(buffer, actions);
//     if (-1 == action)
//         return 0;
// 
//     NextToken(buffer, &args, NULL);
//     if (action > 3)
//     {
//         //check for multiple loadings
//         if (IsInString(buffer, "/"))
//         {
//             char path[1];
//             char *p = strcpy(path, buffer);
//             *(char*)get_delim(p, '/') = 0; // strip "/#"
//             strcpy(buffer, (const char*)strcat((char*)p, ".dll"));
//         }
// 
// 
//         char szBuffer[MAX_PATH];
//         const char *path=plugrcPath();
// 
//         FILE *fp = fopen(path,"rb");
//         if (fp)
//         {
//             if (e_edit == action)
//                 while (read_next_line(fp, szBuffer, sizeof szBuffer))
//                     parse_pluginRC(szBuffer, buffer);
//             else
//                 while (read_next_line(fp, szBuffer, sizeof szBuffer))
//                     parse_pluginDocs(szBuffer, buffer);
//             fclose(fp);
//         }
//     }
// 
//     if (e_add == action && 0 == buffer[0])
//     {
// #ifndef BBTINY
//         if (false == browse_file(buffer,
//             NLS1("Add Plugin"),
//             "Plugins (*.dll)\0*.dll\0All Files (*.*)\0*.*\0"))
// #endif
//             return 1;
//     }
// 
//     strcpy(buffer, get_relative_path(NULL, unquote(buffer)));
// 
//     if (e_add == action) {
//         q = parseConfigLine(buffer);
//         append_node(&bbplugins, q);
// 
//         q->useSlit = true;
//     } else {
//         skipUntil(q, bbplugins, q->name && !_stricmp(q->name, buffer));
//     }
// 
//     if (q == NULL)
//         return 1;
// 
//     if (e_remove == action)
//         q->isEnabled = false;
//     else if (e_load == action)
//     {
//         if(get_false_true(args) == -1)
//             q->isEnabled ^= true;
//         else
//             q->isEnabled = !!get_false_true(args);
//     }
//     else if (e_inslit == action)
//     {
//         if(get_false_true(args) == -1)
//             q->useSlit ^= true;
//         else
//             q->useSlit = !!get_false_true(args);
//     }
// 
//     applyPluginStates();
// 
//     if (e_remove == action || (e_add == action && !q->isEnabled))
//         free_plugin((struct PluginList **)member_ptr(&bbplugins, q));
// 
//     write_plugins();
// 
//     return 1;
}

}
