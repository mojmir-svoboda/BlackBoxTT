#pragma once
#include "encoder.h"
#include <LogScopeType.h>
#include <LogMsg.h>
#include <sysfn/os.h>

namespace asn1 {

	inline size_t encode_log (char * buff, size_t buff_ln, trace::ScopedLog::E_Type scptype, trace::level_t level, trace::context_t context, char const * file, int line, char const * func, char const * fmt, va_list args)
	{
		Command_t command;
		memset(&command, 0, sizeof(Command));
		command.present = Command_PR_log;

		OCTET_STRING_t os_file = mkOctetString(file);
		OCTET_STRING_t os_func = mkOctetString(func);
		char const * wdgt = "main";
		OCTET_STRING_t os_wdgt = mkOctetString(wdgt);

		sys::hptimer_t now = sys::queryTime_us();
		// @TODO @FIXME uint64_t into asn1c
		command.choice.log.ctime = now;
		command.choice.log.lvl = level;
		command.choice.log.ctx = context;
		command.choice.log.tid = sys::get_tid();
		command.choice.log.file = os_file;
		command.choice.log.line = line;
		command.choice.log.func = os_func;
		if (scptype != trace::ScopedLog::e_None)
			command.choice.log.scope = scptype == trace::ScopedLog::e_Entry ? LogScopeType_scopeEntry : LogScopeType_scopeExit;
		else
			command.choice.log.scope = LogScopeType_scopeNone;
		command.choice.log.wdgt = os_wdgt;

		const size_t need = vsnprintf(nullptr, 0, fmt, args);
		assert(need >= 0);
		char * msg = static_cast<char *>(_alloca(need + 1));
		const size_t msg_sz = vsnprintf(msg, need + 1, fmt, args);
		OCTET_STRING_t os_msg = mkOctetString(msg);

		command.choice.log.msg = os_msg;

		asn_enc_rval_t const ec = der_encode_to_buffer(nullptr, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}
}

