#pragma once
#include "encoder.h"
#include <trace_proto/asn.1/PlotMarkerMsg.h>
#include <sysfn/os.h>

namespace asn1 {

	inline size_t encode_plot_marker (Allocator * allocator, char * buff, size_t buff_ln, trace::level_t level, trace::context_t context, float x, float y, char const * fmt, va_list args)
	{
		Command_t command;
		memset(&command, 0, sizeof(Command));
		command.present = Command_PR_plotm;

		sys::hptimer_t now = sys::queryTime_us();
		// @TODO @FIXME uint64_t into asn1c
		command.choice.plot.ctime = now;
		command.choice.plot.lvl = level;
		command.choice.plot.ctx = context;
		command.choice.plot.x = x;
		command.choice.plot.y = y;
		const size_t need = vsnprintf(nullptr, 0, fmt, args);
		assert(need >= 0);
		char * msg = static_cast<char *>(_alloca(need + 1));
		const size_t msg_sz = vsnprintf(msg, need + 1, fmt, args);
		OCTET_STRING_t os_msg = mkOctetString(msg);
		command.choice.plot.wdgt = os_msg;

		asn_enc_rval_t const ec = der_encode_to_buffer(allocator, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}
}

