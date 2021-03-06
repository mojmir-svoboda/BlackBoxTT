#include "Scheme.h"
#include <blackbox/common.h>
#include <3rd_party/s7/s7.h>
#include <blackbox/bind/bind.h>
#include <BlackBox.h>

//typedef s7_pointer(*s7_function)(s7_scheme *sc, s7_pointer args);   /* that is, obj = func(s7, args) -- args is a list of arguments */
bbstring toString (char const * src)
{
	size_t const ln = strlen(src);
	size_t const sz = bb::codecvt_utf8_utf16_dst_size(src, ln);
	wchar_t * const bbcmd_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
	size_t const bbcmd_u16_ln = bb::codecvt_utf8_utf16(src, ln, bbcmd_u16, sz);
	bbstring b(bbcmd_u16, bbcmd_u16_ln);
	return std::move(b);
}

s7_pointer bind_SaveConfig (s7_scheme * sc, s7_pointer args)
{
// 	bb::BlackBox * const bb = getBlackBoxInstanceRW();
// 	bb->SaveConfig();

	return s7_nil(sc);
}

s7_pointer bind_LoadPlugin (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * plugin_id = s7_string(s7_car(args));
		bbstring name = toString(plugin_id);
		getBlackBoxInstanceRW()->LoadPlugin(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "LoadPlugin", 1, s7_car(args), "utf8 string");
}
s7_pointer bind_UnloadPlugin (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * plugin_id = s7_string(s7_car(args));
		bbstring name = toString(plugin_id);
		getBlackBoxInstanceRW()->UnloadPlugin(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "UnloadPlugin", 1, s7_car(args), "utf8 string");
}
s7_pointer bind_IsPluginLoaded (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * widget_name = s7_string(s7_car(args));
		bbstring name = toString(widget_name);
		bb::BlackBox * const bb = getBlackBoxInstanceRW();

		bool const loaded = bb->IsPluginLoaded(name);
		return s7_make_boolean(sc, loaded);
	}
	return s7_wrong_type_arg_error(sc, "IsPluginLoaded", 1, s7_car(args), "utf8 string id of plugin from config section Plugins");
}

s7_pointer bind_ShowExplorer (s7_scheme * sc, s7_pointer args)
{
	getBlackBoxInstanceRW()->ShowExplorer();
	return s7_nil(sc);
}
s7_pointer bind_HideExplorer (s7_scheme * sc, s7_pointer args)
{
	getBlackBoxInstanceRW()->HideExplorer();
	return s7_nil(sc);
}
s7_pointer bind_IsExplorerVisible (s7_scheme * sc, s7_pointer args)
{
	bb::BlackBox * const bb = getBlackBoxInstanceRW();
	bool const loaded = bb->IsExplorerVisible();
	return s7_make_boolean(sc, loaded);
}

s7_pointer bind_SetQuit (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_integer(s7_car(args)))
	{
		uint32_t const n = s7_integer(s7_car(args));
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->Quit(n);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetQuit", 1, s7_car(args), "integer code");
}


s7_pointer bind_CreateWidgetFromId (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * widget_name = s7_string(s7_car(args));
		bbstring name = toString(widget_name);
		getBlackBoxInstanceRW()->CreateWidgetFromId(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "CreateWidgetFromId", 1, s7_car(args), "utf8 string id in config section [Widgets]");
}

s7_pointer bind_ToggleDesktopMenu (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * widget_name = s7_string(s7_car(args));
		bbstring name = toString(widget_name);
		getBlackBoxInstanceRW()->ToggleDesktopMenu(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "ToggleDesktopMenu", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_SetCurrentVertexId (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * vertex = s7_string(s7_car(args));
		bbstring name = toString(vertex);
		getBlackBoxInstanceRW()->WorkSpacesSetCurrentVertexId(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_SwitchVertexViaEdge (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * edge = s7_string(s7_car(args));
		bbstring name = toString(edge);
		getBlackBoxInstanceRW()->WorkSpacesSwitchVertexViaEdge(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_MaximizeTopWindow (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * arg = s7_string(s7_car(args));
		bbstring a = toString(arg);
		bool vertical = a == L"vertical";
		getBlackBoxInstanceRW()->MaximizeTopWindow(vertical);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_MoveTopWindowToVertexViaEdge (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * param = s7_string(s7_car(args));
		bbstring name = toString(param);
		getBlackBoxInstanceRW()->MoveTopWindowToVertexViaEdge(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_SetTaskManIgnored (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * param = s7_string(s7_car(args));
		bbstring name = toString(param);
		getBlackBoxInstanceRW()->SetTaskManIgnored(name);
		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetTaskManIgnored", 1, s7_car(args), "utf8 string");
}

namespace bb {

	Scheme::Scheme ()
		: m_scheme(nullptr)
	{ }
	Scheme::~Scheme () { }
	
	bool Scheme::Init (SchemeConfig const & cfg)
	{
		m_config = cfg;
		m_scheme = s7_init();

		s7_define_function(m_scheme, "SetQuit", bind_SetQuit, 1, 0, false, "(SetQuit int) Quits with code int");
		s7_define_function(m_scheme, "SaveConfig", bind_SaveConfig, 0, 0, false, "(SaveConfig) Saves main bbTT config");
		s7_define_function(m_scheme, "LoadPlugin", bind_LoadPlugin, 1, 0, false, "(LoadPlugin plugin_id) Load plugin with id from config section [Plugins]");
		s7_define_function(m_scheme, "UnloadPlugin", bind_UnloadPlugin, 1, 0, false, "(UnloadPlugin plugin_id) Unload plugin with id from config section [Plugins]");
		s7_define_function(m_scheme, "IsPluginLoaded", bind_IsPluginLoaded, 1, 0, false, "(IsPluginLoaded plugin_id) returns true if plugin is loaded");
		s7_define_function(m_scheme, "ShowExplorer", bind_ShowExplorer, 0, 0, false, "(ShowExplorer) Show explorer taskbar");
		s7_define_function(m_scheme, "HideExplorer", bind_HideExplorer, 0, 0, false, "(Explorer) Hide explorer taskbar");
		s7_define_function(m_scheme, "IsExplorerVisible", bind_IsExplorerVisible, 0, 0, false, "(IsExplorerVisible) returns true if explorer taskbar visible");
		s7_define_function(m_scheme, "CreateWidgetFromId", bind_CreateWidgetFromId, 1, 0, false, "(CreateWidgetFromId widget_id) Creates widget from id (id from yaml config)");
		s7_define_function(m_scheme, "ToggleDesktopMenu", bind_ToggleDesktopMenu, 1, 0, false, "(ToggleDesktopMenu widget_name) Show/Hide menu with widget_name if hidden/shown");
		s7_define_function(m_scheme, "SetCurrentVertexId", bind_SetCurrentVertexId, 1, 0, false, "(SetCurrentVertexId vertex_id_string) Sets WorkSpace Graph to specified VertexId");
		s7_define_function(m_scheme, "SwitchVertexViaEdge", bind_SwitchVertexViaEdge, 1, 0, false, "(SwitchVertexViaEdge edge_id_string) Switch WorkSpace via edge to destination vertex_id");
		s7_define_function(m_scheme, "MaximizeTopWindow", bind_MaximizeTopWindow, 1, 0, false, "(SwitchVertexViaEdge edge_id_string) Switch WorkSpace via edge to destination vertex_id");
		s7_define_function(m_scheme, "MoveTopWindowToVertexViaEdge", bind_MoveTopWindowToVertexViaEdge, 1, 0, false, "(MoveTopWindowToVertexViaEdge edge_id_string) Move focused window via edge to destination vertex_id");
		s7_define_function(m_scheme, "SetTaskManIgnored", bind_SetTaskManIgnored, 1, 0, false, "(SetTaskManIgnored type) Sets/Toggles/Unsets task manager ignore flag on task. Ignored tasks are not shown in Alt-Tab");

		//s7_define_function(s7, "add1", add1, 1, 0, false, "(add1 int) adds 1 to int");
/* add the function "add1" to the interpreter.
*		1, 0, false -> one required arg,
*									 no optional args,
*									 no "rest" arg
*/
		//s7_define_variable(s7, "my-pi", s7_make_real(s7, 3.14159265));

		TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "Scheme init");
		return true;
	}

	bool Scheme::Eval (char const * str, char * resp, size_t resp_sz)
	{
		s7_pointer res = s7_eval_c_string(m_scheme, str);
		if (resp)
		{
			char * ptr = s7_object_to_c_string(m_scheme, res);
			snprintf(resp, resp_sz, "%s", ptr);
			free(ptr);
			ptr = nullptr;
		}
		return true;
	}
 
	bool Scheme::Done ()
	{
		free(m_scheme);
		m_scheme = nullptr;
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Scheme terminating...");
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Scheme terminated.");
		return true;
	}
}

