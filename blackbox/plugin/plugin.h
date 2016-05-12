#pragma once
#include <platform_win.h>

using beginPlugin_t     = int (*) (HINSTANCE);
using beginPluginEx_t   = int (*) (HINSTANCE, HWND);
using beginSlitPlugin_t = int (*) (HINSTANCE, HWND);
using endPlugin_t       = int (*) (HINSTANCE);
using pluginInfo_t      = char const * (*) (int);
using pluginInfoW_t      = wchar_t const * (*) (int);

char const c_beginPluginName[] = "beginPlugin";
char const c_beginPluginExName[] = "beginPluginEx";
char const c_beginSlitPluginName[] = "beginSlitPlugin";
char const c_endPluginName[] = "endPlugin";
char const c_pluginInfoName[] = "pluginInfo";
char const c_pluginInfoWName[] = "pluginInfoW";

/* return values for beginPlugin functions */
#define BEGINPLUGIN_OK      0
#define BEGINPLUGIN_FAILED  1
#define BEGINPLUGIN_FAILED_QUIET 2

/* possible index values for pluginInfo */
#define PLUGIN_NAME         1
#define PLUGIN_VERSION      2
#define PLUGIN_AUTHOR       3
#define PLUGIN_RELEASE      4
#define PLUGIN_LINK         5
#define PLUGIN_EMAIL        6


