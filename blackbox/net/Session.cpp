#include "Session.h"
#include "Server.h"
#include <utility>
#include <memory>
#include <platform_win.h>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <bblib/logging.h>
#include <bbproto/decoder.h>
#include "commands.h"
#include <boost/lockfree/spsc_queue.hpp>

namespace bb {

	Session::Session (Server * s, asio::io_context & io, asio::ip::tcp::socket socket)
		: m_server(s)
		, m_io(io)
		, m_socket(std::move(socket))
		, m_responseLock()
		, m_writeStrand(io)
		, m_responses()
		, m_pendingWrite(false)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server session @ 0x%x started, waiting for data", this);
		m_asn1Allocator.resizeStorage(m_asn1Allocator.calcNextSize());
	}

	void Session::Start ()
	{
		DoReadHeader();
	}

	void Session::DoReadHeader ()
	{
		auto self(shared_from_this());
		
		m_decodingCtx.reset();
		m_decodingCtx.resetCurrentCommand();
		m_asn1Allocator.reset();
		size_t const hdr_sz = sizeof(asn1::Header);
		asio::async_read(
			m_socket,
			asio::buffer(m_decodingCtx.begin(), m_decodingCtx.available()),
			asio::transfer_exactly(4),
			[this, self] (std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					DoReadBody();
				}
				else
					m_socket.close();
			});
	}

	void Session::DoReadBody ()
	{
		auto self(shared_from_this());

		size_t const hdr_sz = sizeof(asn1::Header);
		asn1::Header const & hdr = m_decodingCtx.getHeader();
		asio::async_read(
			m_socket,
			asio::buffer(m_decodingCtx.begin() + hdr_sz, m_decodingCtx.available() - hdr_sz),
			asio::transfer_exactly(hdr.m_len),
			[this, self] (std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					bool ok = DoDecodeBody();
					if (!ok)
						m_socket.close();

					DoReadHeader();
				}
				else
					m_socket.close();
			});
	}

	bool Session::DoDecodeBody ()
	{
		::Command * cmd_ptr = &m_decodingCtx.m_command;
		void * cmd_void_ptr = cmd_ptr;
		char const * payload = m_decodingCtx.getPayload();
		asn1::Header const & hdr = m_decodingCtx.getHeader();
		size_t const av = m_asn1Allocator.available();
		size_t const size_estimate = hdr.m_len * 4; // at most
		//sys::hptimer_t const now = sys::queryTime_us();
		//m_dcd_ctx.m_command.m_stime = now;
		if (av < size_estimate)
		{
			// not enough memory for asn1 decoder (**)
			// 1) flush everything
			// 2) resize 
			m_asn1Allocator.Reset();
			m_asn1Allocator.resizeStorage(m_asn1Allocator.calcNextSize());
			// 3) check
			size_t const av = m_asn1Allocator.available();
			if (av < size_estimate)
			{
				return false;
			}
		}

		const asn_dec_rval_t rval = ber_decode(&m_asn1Allocator, 0, &asn_DEF_Command, &cmd_void_ptr, payload, hdr.m_len);
		if (rval.code != RC_OK)
		{
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_NET, "Decoder exception: Error while decoding ASN1: err=%u, consumed %u bytes", rval.code, rval.consumed);

			m_decodingCtx.resetCurrentCommand();
			m_decodingCtx.reset();
			m_asn1Allocator.Reset();
			return false;
		}

		QueueDecodedRequest(m_decodingCtx.m_command);
		return true;
	}

	bool Session::QueueDecodedRequest (DecodedCommand const & cmd)
	{
		std::unique_ptr<Command> c = mkCommand(cmd);
		if (c)
		{
			m_server->AddRequest(std::move(c), shared_from_this());
			return true;
		}
		return false;
	}

	bool Session::AddResponse (std::unique_ptr<PendingCommand> cmd)
	{
		m_writeStrand.post(
				m_io, 
				[this, cmd] ()
				{
					//m_responseLock.Lock();
					bool const write_in_progress = !m_responses.empty();
					m_responses.enqueue(std::move(cmd));
					//m_responseLock.Unlock();

					if (!write_in_progress)
						DoWriteResponse ();
				});

	}

	void Session::DoWriteResponse ()
	{
		m_pendingWrite = true;


	}
}

