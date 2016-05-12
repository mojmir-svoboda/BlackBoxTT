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

	trace::DictionaryPair lvl_dict[]{
		{ (1u << LL_VERBOSE), "verbose" },
		{ (1u << LL_DEBUG), "dbg" },
		{ (1u << LL_INFO), "nfo" },
		{ (1u << LL_WARNING), "WARN" },
		{ (1u << LL_ERROR), "ERROR" },
		{ (1u << LL_FATAL), "FATAL" },
	};
	static_assert(sizeof(lvl_dict) / sizeof(*lvl_dict) == e_max_trace_level, "arrays do not match");
	TRACE_SET_LEVEL_DICTIONARY(lvl_dict, sizeof(lvl_dict) / sizeof(*lvl_dict));

	trace::DictionaryPair ctx_dict[] = {
		{ CTX_DEFAULT 			,  "dflt" },
		{ CTX_INIT 					,  "Init" },
		{ CTX_BB						,  "BB" },
		{ CTX_GFX 					,  "Gfx" },
		{ CTX_EXPLORER 			,  "Explr" },
		{ CTX_HOOK  				,  "Hook" },
		{ CTX_TRAY 					,  "Tray" },
		{ CTX_CONFIG				,  "Cfg" },
		{ CTX_SCRIPT				,  "Script" },
		{ CTX_STYLE					,  "Style" },
		{ CTX_PROFILING 		,  "Prof" },
		{ CTX_SERIALIZE 		,  "Ser" },
		{ CTX_MEMORY 				,  "Memory" },
		{ CTX_RESOURCES			,  "Rsrc" },
		{ CTX_BBLIB					,  "Lib" },
		{ CTX_BBLIBCOMPAT		,  "LibCompat" },
		{ CTX_PLUGINMGR			,  "PlugMgr" },
		{ CTX_PLUGIN 				,  "Plug" },
	};
	TRACE_SET_CONTEXT_DICTIONARY(ctx_dict, sizeof(ctx_dict) / sizeof(*ctx_dict));

	// 2) connect
	TRACE_CONNECT(addr, port);

	TRACE_MSG(LL_INFO, CTX_INIT, "%s connected to server.", appName);
}
#endif

}