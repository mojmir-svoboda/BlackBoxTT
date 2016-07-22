#pragma once
#include "encoder.h"
#include <trace_proto/Command.h>

namespace asn1 {

	inline size_t encode_config (char * buff, size_t buff_ln, char const * appname, char const * mixer, size_t mixer_sz, bool buffered, unsigned pid)
	{
		Command_t command;
		command.present = Command_PR_config;
		//ConfigMsg c;
		OCTET_STRING_t os_app = mkOctetString(appname);
		command.choice.config.app = os_app;
		size_t const sz = mixer_sz;
		OCTET_STRING_t mix_cfg = mkOctetStringRaw(reinterpret_cast<char const *>(mixer), mixer_sz);
		command.choice.config.mixer = mix_cfg;
		command.choice.config.buffered = buffered;
		command.choice.config.pid = pid;

		asn_enc_rval_t const ec = der_encode_to_buffer(nullptr, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}
}

