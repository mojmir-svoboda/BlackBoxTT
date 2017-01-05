#include "Scheme.h"
#include <bblib/logging.h>
#include <3rd_party/s7/s7.h>
#include <blackbox/bind/bind.h>
#include <bblib/codecvt.h>
#include <bblib/bbstring.h>
#include <BlackBox.h>

//typedef s7_pointer(*s7_function)(s7_scheme *sc, s7_pointer args);   /* that is, obj = func(s7, args) -- args is a list of arguments */

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

s7_pointer bind_ShowMenu (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_integer(s7_car(args)))
	{
		uint32_t const n = s7_integer(s7_car(args));
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->ShowMenu(n);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "ShowMenu", 1, s7_car(args), "0/1");
}

s7_pointer bind_ToggleMenu (s7_scheme * sc, s7_pointer args)
{
	bb::BlackBox * const bb = getBlackBoxInstanceRW();
	bb->ToggleMenu();
	return s7_nil(sc);
}

s7_pointer bind_SetCurrentVertexId (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * vertex = s7_string(s7_car(args));

		size_t const ln = strlen(vertex);
		size_t const sz = bb::codecvt_utf8_utf16_dst_size(vertex, ln);
		wchar_t * const bbcmd_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
		size_t const bbcmd_u16_ln = bb::codecvt_utf8_utf16(vertex, ln, bbcmd_u16, sz);
		bbstring b(bbcmd_u16, bbcmd_u16_ln);

		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->WorkSpacesSetCurrentVertexId(b);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_SwitchVertexViaEdge (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * vertex = s7_string(s7_car(args));

		size_t const ln = strlen(vertex);
		size_t const sz = bb::codecvt_utf8_utf16_dst_size(vertex, ln);
		wchar_t * const bbcmd_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
		size_t const bbcmd_u16_ln = bb::codecvt_utf8_utf16(vertex, ln, bbcmd_u16, sz);
		bbstring edge_id(bbcmd_u16, bbcmd_u16_ln);

		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->WorkSpacesSwitchVertexViaEdge(edge_id);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_MaximizeTopWindow (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * param = s7_string(s7_car(args));

		size_t const ln = strlen(param);
		size_t const sz = bb::codecvt_utf8_utf16_dst_size(param, ln);
		wchar_t * const param_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
		size_t const param_u16_ln = bb::codecvt_utf8_utf16(param, ln, param_u16, sz);
		bbstring p(param_u16, param_u16_ln);

		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bool vertical = p == L"vertical";
		bb->MaximizeTopWindow(vertical);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_MoveTopWindowToVertexViaEdge (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * param = s7_string(s7_car(args));

		size_t const ln = strlen(param);
		size_t const sz = bb::codecvt_utf8_utf16_dst_size(param, ln);
		wchar_t * const param_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
		size_t const param_u16_ln = bb::codecvt_utf8_utf16(param, ln, param_u16, sz);
		bbstring p(param_u16, param_u16_ln);

		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->MoveTopWindowToVertexViaEdge(p);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");
}

s7_pointer bind_SetTaskManIgnored (s7_scheme * sc, s7_pointer args)
{
	if (s7_is_string(s7_car(args)))
	{
		char const * param = s7_string(s7_car(args));

		size_t const ln = strlen(param);
		size_t const sz = bb::codecvt_utf8_utf16_dst_size(param, ln);
		wchar_t * const param_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
		size_t const param_u16_ln = bb::codecvt_utf8_utf16(param, ln, param_u16, sz);
		bbstring p(param_u16, param_u16_ln);

		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->SetTaskManIgnored(p);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetTaskManIgnored", 1, s7_car(args), "utf8 string");
}


namespace bb {

	Scheme::Scheme ()
		: m_scheme(nullptr)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Scheme @ 0x%x", this);
	}
	Scheme::~Scheme ()
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "~Scheme @ 0x%x", this);
	}
	
	bool Scheme::Init (SchemeConfig const & cfg)
	{
		m_config = cfg;
		m_scheme = s7_init();

		s7_define_function(m_scheme, "SetQuit", bind_SetQuit, 1, 0, false, "(SetQuit int) Quits with code int");
		s7_define_function(m_scheme, "ShowMenu", bind_ShowMenu, 1, 0, false, "(ShowMenu int) Show/Hide menu where int=1/0");
		s7_define_function(m_scheme, "ToggleMenu", bind_ToggleMenu, 0, 0, false, "(ToggleMenu) Show/Hide menu if hidden/shown");
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

