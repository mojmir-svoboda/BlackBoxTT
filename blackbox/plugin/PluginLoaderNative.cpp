#include "../BB.h"
#include "../BBApiPluginloader.h"

#include "PluginManager.h"
#include "PluginLoaderNative.h"
#include "Types.h"
#include <lib2/bblib2.h>

char name[255];

struct PluginLoaderList nativeLoader = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		Init,
	Finalize,
	
		GetName,
	GetApi,
		GetPluginInfo,

	LoadPlugin,
	UnloadPlugin
};

bool Init(char* workingDir) {
		sprintf(name, "Core Pluginloader %s", GetBBVersion());
		return true;
}

void Finalize() {}

const char *GetName() {
		return name;
}

const char *GetApi() {
		return GetBBVersion();
}

const char *GetPluginInfo(struct PluginList* plugin, int factId) {
		struct NativePluginInfo* pInfo = (struct NativePluginInfo*)plugin->loaderInfo;

		if(pInfo == NULL || pInfo->pluginInfo == NULL)
				return NULL;

		return pInfo->pluginInfo(factId);
}

int LoadPlugin(struct PluginList* q, HWND hSlit, char** errorMsg) {
		struct NativePluginInfo* pInfo = c_new<struct NativePluginInfo>();
		int error = 0;
		bool useslit;
		int r;
		char plugin_path[MAX_PATH];

		for (;;)
		{
				//---------------------------------------
				// load the dll
				if (0 == FindRCFile(plugin_path, q->path, NULL)) {
						error = error_plugin_dll_not_found;
						break;
				}

				r = SetErrorMode(0); // enable 'missing xxx.dll' system message
				pInfo->module = LoadLibrary(plugin_path);
				SetErrorMode(r);

				if (NULL == pInfo->module)
				{
						r = GetLastError();
						// char buff[200]; win_error(buff, sizeof buff);
						// dbg_printf("LoadLibrary::GetLastError %d: %s", r, buff);
						if (ERROR_MOD_NOT_FOUND == r)
								error = error_plugin_dll_needs_module;
						else
								error = error_plugin_does_not_load;
						break;
				}

				//---------------------------------------
				// grab interface functions
				LoadFunction(pInfo, beginPlugin);
				LoadFunction(pInfo, beginPluginEx);
				LoadFunction(pInfo, beginSlitPlugin);
				LoadFunction(pInfo, endPlugin);
				LoadFunction(pInfo, pluginInfo);

				//---------------------------------------
				// check interfaces
				if(!pInfo->endPlugin)
						BreakWithCode(error, error_plugin_missing_entry);
				
				// check whether plugin supports the slit
				q->canUseSlit = !!pInfo->beginPluginEx || !!pInfo->beginSlitPlugin;
				
				if(!q->canUseSlit)
						q->useSlit = false;

				useslit = hSlit && q->useSlit;

				if (!useslit && !pInfo->beginPluginEx && !pInfo->beginPlugin)
						BreakWithCode(error, error_plugin_missing_entry);

				//---------------------------------------
				// inititalize plugin
				TRY
				{
						if (useslit) {
								if (pInfo->beginPluginEx)
										r = pInfo->beginPluginEx(pInfo->module, hSlit);
								else
										r = pInfo->beginSlitPlugin(pInfo->module, hSlit);
						} else {
								if (pInfo->beginPlugin)
										r = pInfo->beginPlugin(pInfo->module);
								else
										r = pInfo->beginPluginEx(pInfo->module, NULL);
						}

						if (BEGINPLUGIN_OK == r) {
								q->loaderInfo = pInfo;
								q->inSlit = useslit;
						} else if (BEGINPLUGIN_FAILED_QUIET != r) {
								error = error_plugin_fail_to_load;
						}

				}
				EXCEPT(EXCEPTION_EXECUTE_HANDLER)
				{
						error = error_plugin_crash_on_load;
				}
				break;
		}

		// clean up after error
		if(error) {
				if (pInfo->module)
						FreeLibrary(pInfo->module);

				q->loaderInfo = NULL;
				c_del(pInfo);
		}

		return error;
}

int UnloadPlugin(struct PluginList* q, char** errorMsg) {
		struct NativePluginInfo* pi = (struct NativePluginInfo*)(q->loaderInfo);
		
		int error = 0;
		if (pi != NULL && pi->module) {
				TRY {
						pi->endPlugin(pi->module);
				} EXCEPT(EXCEPTION_EXECUTE_HANDLER) {
						error = error_plugin_crash_on_unload;
				}

				FreeLibrary(pi->module);
				pi->module = NULL;
				
				q->loaderInfo = NULL;
				c_del(pi);
		}

		return error;
}
