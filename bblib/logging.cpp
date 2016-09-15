#include "logging.h"

namespace bb {

#if defined TRACE_ENABLED
void initTrace (char const * appName, char const * addr, char const * port)
{
	// 1) setup app_name, buffering, starting levels, level and context dictionaries
	TRACE_APPNAME(appName);
	TRACE_SETBUFFERED(0);

	const trace::context_t all_contexts = -1;
	const trace::level_t errs = (1u << LL_ERROR) | (1u << LL_FATAL);
	TRACE_SET_LEVEL(all_contexts, errs);
	const trace::level_t normal_lvl = (1u << LL_INFO) | (1u << LL_WARNING);
	TRACE_SET_LEVEL(CTX_DEFAULT | CTX_INIT, normal_lvl);
	const trace::level_t all_lvl = (1u << LL_VERBOSE) | (1u << LL_DEBUG) | (1u << LL_INFO) | (1u << LL_WARNING) | (1u << LL_ERROR) | (1u << LL_FATAL);
	TRACE_SET_LEVEL(CTX_BB, all_lvl);

	//TRACE_SET_LEVEL(CTX_NET_MESSAGE | CTX_NETWORK, all_lvl);

	trace::level_t lvl_dict_values[] = {
		(1u << LL_VERBOSE),
		(1u << LL_DEBUG),
		(1u << LL_INFO),
		(1u << LL_WARNING),
		(1u << LL_ERROR),
		(1u << LL_FATAL)
	};
	static_assert(sizeof(lvl_dict_values) / sizeof(*lvl_dict_values) == e_max_trace_level, "array do not match enum");
	char const * lvl_dict_names[] = {
		"vrbs",
		"dbg",
		"nfo",
		"WARN",
		"ERROR",
		"FATAL"
	};
	static_assert(sizeof(lvl_dict_names) / sizeof(*lvl_dict_names) == e_max_trace_level, "array do not match enum");
	TRACE_SET_LEVEL_DICTIONARY(lvl_dict_values, lvl_dict_names, sizeof(lvl_dict_values) / sizeof(*lvl_dict_values));

	trace::context_t ctx_dict_values[] = {
		CTX_DEFAULT 			,
		CTX_INIT 					,
		CTX_BB						,
		CTX_GFX 					,
		CTX_EXPLORER 			,
		CTX_HOOK  				,
		CTX_TRAY 					,
		CTX_CONFIG				,
		CTX_SCRIPT				,
		CTX_STYLE					,
		CTX_PROFILING 		,
		CTX_SERIALIZE 		,
		CTX_MEMORY 				,
		CTX_RESOURCES			,
		CTX_BBLIB					,
		CTX_BBLIBCOMPAT		,
		CTX_PLUGINMGR			,
		CTX_PLUGIN
	};
	char const * ctx_dict_names[] = {
		"dflt",
		"Init",
		"BB",
		"Gfx",
		"Explr",
		"Hook",
		"Tray",
		"Cfg",
		"Script",
		"Style",
		"Prof",
		"Ser",
		"Memory",
		"Rsrc",
		"Lib",
		"LibCompat",
		"PlugMgr",
		"Plug"
	};
	static_assert(sizeof(ctx_dict_values) / sizeof(*ctx_dict_values) == sizeof(ctx_dict_names) / sizeof(*ctx_dict_names), "arrays do not match");
	TRACE_SET_CONTEXT_DICTIONARY(ctx_dict_values, ctx_dict_names, sizeof(ctx_dict_values) / sizeof(*ctx_dict_values));

	// 2) connect
	TRACE_CONNECT(addr, port);

	TRACE_MSG(LL_INFO, CTX_INIT, "%s connected to server.", appName);
}
#endif

}
