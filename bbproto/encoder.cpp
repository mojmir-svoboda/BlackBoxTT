#include "encoder.h"
#include <Command.h>
#include "Header.h"
#include <bblib/codecvt.h>

namespace asn1 {

// 	inline size_t encode_bb32wm (Allocator * allocator, char * buff, size_t buff_ln, unsigned wm)
// 	{
// 		Command_t command;
// 		memset(&command, 0, sizeof(Command));
// 		command.present = Command_PR_bb32wm;
// 		command.choice.bb32wm.wmmsg = wm;
// 
// 		asn_enc_rval_t const ec = der_encode_to_buffer(allocator, &asn_DEF_Command, &command, buff, buff_ln);
// 		if (ec.encoded == -1)
// 			return 0;
// 		return ec.encoded;
// 	}

	inline size_t encode_bbcmd (Allocator * allocator, char * buff, size_t buff_ln, char const * bbcmd, size_t bbcmdln)
	{
		Command_t command;
		memset(&command, 0, sizeof(Command));
		command.present = Command_PR_bbcmd;
		OCTET_STRING_t os_cmd = mkOctetString(bbcmd, bbcmdln);
		command.choice.bbcmd.cmd = os_cmd;

		asn_enc_rval_t const ec = der_encode_to_buffer(allocator, &asn_DEF_Command, &command, buff, buff_ln);
		if (ec.encoded == -1)
			return 0;
		return ec.encoded;
	}

	inline size_t encode_bbcmd (Allocator * allocator, char * buff, size_t buff_ln, wchar_t const * bbcmd, size_t bbcmdln)
	{
		Command_t command;
		memset(&command, 0, sizeof(Command));
		command.present = Command_PR_bbcmd;
		// to utf8
		size_t const sz = bb::codecvt_utf16_utf8_dst_size(bbcmd, bbcmdln);
		char * const bbcmd_u8 = static_cast<char *>(alloca(sz * sizeof(char)));
		size_t const bbcmd_u8_ln = bb::codecvt_utf16_utf8(bbcmd, bbcmdln, bbcmd_u8, sz);
		return encode_bbcmd(allocator, buff, buff_ln, bbcmd_u8, bbcmd_u8_ln);
	}
}

namespace bb {

// 	size_t encode_bb32wm (char * buff, size_t buff_ln, unsigned wm)
// 	{
// 		asn1::Header & hdr = asn1::encode_header(buff, buff_ln);
// 		asn1::Asn1StackAllocator a;
// 		const size_t n = asn1::encode_bb32wm(&a, buff + sizeof(asn1::Header), buff_ln - sizeof(asn1::Header), wm);
// 		hdr.m_len = n;
// 		return n + sizeof(asn1::Header);
// 	}

	size_t encode_bbcmd (char * buff, size_t buff_ln, wchar_t const * bbcmd, size_t bbcmdln)
	{
		asn1::Header & hdr = asn1::encode_header(buff, buff_ln);
		asn1::Asn1StackAllocator a;
		const size_t n = asn1::encode_bbcmd(&a, buff + sizeof(asn1::Header), buff_ln - sizeof(asn1::Header), bbcmd, bbcmdln);
		hdr.m_len = n;
		return n + sizeof(asn1::Header);
	}
	size_t encode_bbcmd (char * buff, size_t buff_ln, char const * bbcmd, size_t bbcmdln)
	{
		asn1::Header & hdr = asn1::encode_header(buff, buff_ln);
		asn1::Asn1StackAllocator a;
		const size_t n = asn1::encode_bbcmd(&a, buff + sizeof(asn1::Header), buff_ln - sizeof(asn1::Header), bbcmd, bbcmdln);
		hdr.m_len = n;
		return n + sizeof(asn1::Header);
	}

}

