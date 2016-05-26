#include "encoder.h"
#include <Command.h>
#include "Header.h"

namespace asn1 {

	inline size_t encode_bb32wm (Allocator * allocator, char * buff, size_t buff_ln, unsigned wm)
	{
		Command_t command;
		memset(&command, 0, sizeof(Command));
		command.present = Command_PR_bb32wm;
		command.choice.bb32wm.wmmsg = wm;

		asn_enc_rval_t const ec = der_encode_to_buffer(allocator, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}

	inline size_t encode_bb32wm_ack (Allocator * allocator, char * buff, size_t buff_ln, HANDLE hwnd)
	{
		Command_t command;
		memset(&command, 0, sizeof(Command));
		command.present = Command_PR_bb32wmack;
		command.choice.bb32wm.wmmsg = reinterpret_cast<intptr_t>(hwnd);

		asn_enc_rval_t const ec = der_encode_to_buffer(allocator, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}
}

namespace bb {

	size_t encode_bb32wm (char * buff, size_t buff_ln, unsigned wm)
	{
		asn1::Header & hdr = asn1::encode_header(buff, buff_ln);
		asn1::Asn1StackAllocator a;
		const size_t n = asn1::encode_bb32wm(&a, buff + sizeof(asn1::Header), buff_ln - sizeof(asn1::Header), wm);
		hdr.m_len = n;
		return n + sizeof(asn1::Header);
	}

	size_t encode_bb32wm_ack (char * buff, size_t buff_ln, HANDLE hwnd)
	{
		asn1::Header & hdr = asn1::encode_header(buff, buff_ln);
		asn1::Asn1StackAllocator a;
		const size_t n = asn1::encode_bb32wm_ack(&a, buff + sizeof(asn1::Header), buff_ln - sizeof(asn1::Header), hwnd);
		hdr.m_len = n;
		return n + sizeof(asn1::Header);
	}
}

