#include "logging.h"

namespace bb {

#if defined TRACE_ENABLED
void initTrace (char const * appName, char const * addr, char const * port)
{
	// 1) setup app_name, buffering, starting levels, level and context dictionaries
	TRACE_APPNAME(appName);
	TRACE_SETBUFFERED(0);

	const trace::context_t all_contexts = -1;
	const trace::level_t errs = LL_ERROR | LL_FATAL;
	TRACE_SET_LEVEL(all_contexts, errs);
	const trace::level_t normal_lvl = LL_INFO | LL_WARNING;
	TRACE_SET_LEVEL(CTX_TASKS | CTX_WSPACE | CTX_INIT | CTX_NET | CTX_GFX, normal_lvl);
	const trace::level_t all_lvl = LL_VERBOSE | LL_DEBUG | LL_INFO | LL_WARNING | LL_ERROR | LL_FATAL;
	TRACE_SET_LEVEL(CTX_BB, all_lvl);

	trace::level_t lvl_dict_values[] = {
		LL_VERBOSE,
		LL_DEBUG,
		LL_INFO,
		LL_WARNING,
		LL_ERROR,
		LL_FATAL
	};
	char const * lvl_dict_names[] = {
		"vrbs",
		"dbg",
		"nfo",
		"WARN",
		"ERROR",
		"FATAL"
	};
	static_assert(sizeof(lvl_dict_names) / sizeof(*lvl_dict_names) == sizeof(lvl_dict_values) / sizeof(*lvl_dict_values), "arrays do not match");
	TRACE_SET_LEVEL_DICTIONARY(lvl_dict_values, lvl_dict_names, sizeof(lvl_dict_values) / sizeof(*lvl_dict_values));

	trace::context_t ctx_dict_values[] = {
		CTX_INIT,
		CTX_BB,
		CTX_TASKS,
		CTX_WSPACE,
		CTX_NET,
		CTX_GFX,
		CTX_BBLIB,
		CTX_BBLIBCOMPAT,
		CTX_CONFIG,
		CTX_SCRIPT,
		CTX_BIND,
		CTX_RESOURCES,
		CTX_PLUGINMGR,
		CTX_PLUGIN,
	};
	char const * ctx_dict_names[] = {
		"Init",
		"BB",
		"Tasks",
		"Wspace",
		"Net",
		"Gfx",
		"Lib",
		"LibCompat",
		"Cfg",
		"Script",
		"Bind",
		"Rsrcs",
		"PluginMgr",
		"Plugin"
	};
	static_assert(sizeof(ctx_dict_values) / sizeof(*ctx_dict_values) == sizeof(ctx_dict_names) / sizeof(*ctx_dict_names), "arrays do not match");
	TRACE_SET_CONTEXT_DICTIONARY(ctx_dict_values, ctx_dict_names, sizeof(ctx_dict_values) / sizeof(*ctx_dict_values));

	// 2) connect
	TRACE_CONNECT(addr, port);

	TRACE_MSG(LL_INFO, CTX_INIT, "%s connected to server.", appName);
}
#endif

}
