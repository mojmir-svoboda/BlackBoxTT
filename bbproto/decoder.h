#pragma once
#include "Header.h"
#include <Command.h>
#include "decoder_alloc.h"
#include "membuffer.h"

struct DecodedCommand : Command
{
	uint64_t m_stime;
};

struct DecodingContext : MemBuffer<HeapBuffer<16384>>
{
	bool m_has_hdr;
	bool m_has_payload;
	DecodedCommand m_command;

	DecodingContext ()
		: m_has_hdr(false), m_has_payload(false)
	{
		resizeStorage(16384);
		memset(&m_command, 0, sizeof(DecodedCommand));
	}

	char * getEndPtr () { return end(); }
	void moveEndPtr (size_t n)
	{
		moveEnd(n);
	}

	asn1::Header const & getHeader () { return *reinterpret_cast<asn1::Header *>(begin()); }
	char * getPayload () { return begin() + sizeof(asn1::Header); }

	size_t getSize () { return m_end_offset; }

	void resetCurrentCommand ()
	{
		m_has_hdr = m_has_payload = false;
		memset(&m_command, 0, sizeof(DecodedCommand));
		m_end_offset = 0;
	}
};
