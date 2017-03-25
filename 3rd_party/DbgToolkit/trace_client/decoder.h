#pragma once
#include "trace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <trace_proto/header.h>
#include <trace_proto/decoder.h>
#define NOMINMAX 1
#include <asio.hpp>
#include <sysfn/time_query.h>
#include <ScopeGuard.h>

namespace trace {

	struct Decoder
	{
		unsigned m_asn1_buffer_sz { 4096 };
		Asn1Allocator m_asn1_allocator;
		DecodingContext m_dcd_ctx;

		Decoder ()
		{
			m_asn1_allocator.resizeStorage(m_asn1_buffer_sz);
		}

		char * getEndPtr () { return m_dcd_ctx.getEndPtr(); }

		bool parseASN1 ()
		{
			asn1::Header const & hdr = m_dcd_ctx.getHeader();
			if (hdr.m_version == 1)
			{
				Command * cmd_ptr = &m_dcd_ctx.m_command;
				void * cmd_void_ptr = cmd_ptr;
				char const * payload = m_dcd_ctx.getPayload();
				size_t const av = m_asn1_allocator.available();
				size_t const size_estimate = hdr.m_len * 4; // at most
				if (av < size_estimate)
				{	// not enough memory for asn1 decoder
					assert(0);
					return false;
				}

				asn_dec_rval_t const rval = ber_decode(&m_asn1_allocator, 0, &asn_DEF_Command, &cmd_void_ptr, payload, hdr.m_len);
				if (rval.code != RC_OK)
				{
					assert(0);
					return false;
				}
			}
			return true;
		}

		void resetDecoder ()
		{
			m_dcd_ctx.resetCurrentCommand();
			m_asn1_allocator.Reset();
		}
	};
}
