#include "Scheme.h"
#include <bblib/logging.h>
#include <3rd_party/s7/s7.h>
#include <blackbox/bind/bind.h>
#include <bblib/codecvt.h>
#include <bblib/bbstring.h>
#include <BlackBox.h>

//typedef s7_pointer(*s7_function)(s7_scheme *sc, s7_pointer args);   /* that is, obj = func(s7, args) -- args is a list of arguments */

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
		bb->GetWorkSpaces().SetCurrentVertexId(b);

		return s7_nil(sc);
	}
	return s7_wrong_type_arg_error(sc, "SetCurrentVertexId", 1, s7_car(args), "utf8 string");

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

		s7_define_function(m_scheme, "SetCurrentVertexId", bind_SetCurrentVertexId, 1, 0, false, "(SetCurrentVertexId vertex_id_string) Sets WorkSpace Graph to specified VertexId");
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

