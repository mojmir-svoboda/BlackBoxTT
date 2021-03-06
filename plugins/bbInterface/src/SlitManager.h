/*===================================================

	SLIT MANAGER HEADER - Copyright 2004 grischka

	- grischka@users.sourceforge.net -

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_SlitManager_h
#define BBInterface_SlitManager_h
#include <blackbox/plugin/plugin.h>

typedef struct PluginInfo
{
	struct PluginInfo *next;

	// info
	HWND hwnd;
	WNDPROC wp;
	HMODULE hMO;
	wchar_t module_name[96];

	// the window sizes
	int width;
	int height;

	// the window position,
	int xpos;
	int ypos;

	bool is_moving;

}  PluginInfo;

typedef struct ModuleInfo
{
	struct ModuleInfo *next;
	HMODULE hMO;
	wchar_t file_name[MAX_PATH];
	wchar_t module_name[96];
	endPlugin_t endPlugin;
	pluginInfoW_t pluginInfo;

} ModuleInfo;


//=============================================================================
// SlitManager functions
ModuleInfo *loadPlugin(ModuleInfo **, HWND hSlit, const wchar_t *module_name);
bool unloadPlugin(ModuleInfo **, const wchar_t *module_name);
int SlitWndProc(control *c, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool plugin_getset_show_state(PluginInfo *PI, wchar_t *module_name, int state);
bool plugin_setpos(PluginInfo *PI, wchar_t *module_name, int x, int y);
void aboutPlugins(ModuleInfo *m, const wchar_t *);

//=============================================================================
#endif
