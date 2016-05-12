#pragma once
#include <platform_win.h>
#include <bbstring.h>
#include <memory>

struct PluginList;
typedef BOOL(*PLUGINENUMPROC)(PluginList const *, LPARAM);
void EnumPlugins(PLUGINENUMPROC lpEnumFunc, LPARAM lParam);
/*
struct PluginLoaderBase
{
  virtual ~PluginLoaderBase () { }

  virtual bool Init (char const * workingDir);

};

struct PluginLoaderList
{
    PluginLoaderList * m_next;
	PluginList * parent;
    
	HMODULE module;
	const char* name;
	const char* api;
		
	struct PluginPtr * plugins;
		
	bool (*Init)(char* workingDir);
	void (*Finalize)();
		
	const char* (*GetName)();
    const char* (*GetApi)();

    const char* (*GetPluginInfo)(struct PluginList* plugin, int factId);

	int (*LoadPlugin)(struct PluginList* plugin, HWND hSlit, char** errorMsg);
	int (*UnloadPlugin)(struct PluginList* plugin, char** errorMsg);
};

static const char * const pluginLoaderFunctionNames[] = {
    "Init",
    "Finalize",
    "GetName",
    "GetApi",
    "LoadPlugin",
    "UnloadPlugin",
    NULL
};

#define LoadFunction(pll, fn) (*(FARPROC*)&pll->fn = (pll->fn) ? ((FARPROC)pll->fn) : (GetProcAddress(pll->module, #fn)))
#define FailWithMsg(msgVar, msg) { if(msgVar) \
                                     *msgVar = msg; \
                                   return error_plugin_message; \
                                 }
#define BreakWithCode(errVar, code) { errVar = code; break; }
*/
