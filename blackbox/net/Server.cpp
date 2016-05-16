#include "Server.h"
#include <utility>
#include <memory>
#include <platform_win.h>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <bblib/logging.h>
#include <bbproto/decoder.h>

namespace bb {

	struct Session : std::enable_shared_from_this<Session>
	{
		asio::ip::tcp::socket m_socket;
		DecodedCommand m_currentCmd;
		DecodingContext m_decodingCtx;
		Asn1Allocator m_asn1Allocator;

		Session (asio::ip::tcp::socket socket)
			: m_socket(std::move(socket))
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server session @ 0x%x started, waiting for data", this);
			m_asn1Allocator.resizeStorage(m_asn1Allocator.calcNextSize());
		}

		void Start ()
		{
			DoReadHeader();
		}

		void DoReadHeader ()
		{
			auto self(shared_from_this());
			
			size_t const hdr_sz = sizeof(asn1::Header);
			asio::async_read(
				m_socket,
				asio::buffer(m_decodingCtx.begin(), m_decodingCtx.available()),
				asio::transfer_exactly(4),
				[this, self] (std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						//m_decodingCtx.moveEnd(length);
						DoReadBody();
					}
					else
						m_socket.close();
				});
		}

		void DoReadBody()
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

						m_decodingCtx.reset();
						m_asn1Allocator.reset();
						DoReadHeader();
					}
					else
						m_socket.close();
				});
		}

		bool DoDecodeBody ()
		{
			Command * cmd_ptr = &m_decodingCtx.m_command;
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
				//Stats::get().m_decoder_mem_asn1_realloc_count++;
				// 1) flush everything
				//emit onHandleCommandsCommit();
				//batch_size = 0;
				//Stats::get().m_received_batches++;
				// 2) resize 
				m_asn1Allocator.Reset();
				//m_dcd_ctx.resetCurrentCommand();
				m_asn1Allocator.resizeStorage(m_asn1Allocator.calcNextSize());
				//Stats::get().updateDecoderMemAsn1Max(m_asn1_allocator.capacity());
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
				//TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker got connection");
				//QMessageBox::critical(0, tr("trace server"), tr("Decoder exception: Error while decoding ASN1: err=%1, consumed %2 bytes").arg(rval.code).arg(rval.consumed), QMessageBox::Ok, QMessageBox::Ok);

				m_decodingCtx.resetCurrentCommand();
				m_asn1Allocator.Reset();
				//Stats::get().m_received_failed_cmds++;
				return false;
			}
			else
			{
				//Stats::get().m_received_cmds++;
			}

			//tryHandleCommand(m_decodingCtx.m_command, e_RecvBatched);

			m_decodingCtx.resetCurrentCommand(); // reset current decoder command for another decoding pass
			m_asn1Allocator.Reset();
			return true;
		}
	};

	struct AsioServer
	{
		AsioServer (asio::io_context & io, unsigned short port)
			: m_acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{
			DoAccept();
		}

		void DoAccept ()
		{
			auto fn = [this] (std::error_code ec, asio::ip::tcp::socket socket)
			{
				TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker got connection");
				if (!ec)
				{
					std::make_shared<Session>(std::move(socket))->Start();
				}

				DoAccept();
			};
			m_acceptor.async_accept(fn);
		}

	private:
		asio::ip::tcp::acceptor m_acceptor;
	};

	bool Server::Run ()
	{
		try
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker started");
			asio::io_context io_context;
			AsioServer s(io_context, m_config.m_port);
			io_context.run();
		}
		catch (std::exception & e)
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker exception: %s", e.what());
			return false;
		}
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker terminated");
		return true;
	}

	bool Server::Init (ServerConfig const & cfg)
	{
		m_config = cfg;
		TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "Server init, port=%u", cfg.m_port);
		m_thread = std::thread(&Server::Run, this);
		return true;
	}

	bool Server::Done ()
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server terminating...");
		// @TODO: terminate asio io ctx
		m_thread.join();
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server terminated.");
		return true;
	}
}

