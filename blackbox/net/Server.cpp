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

namespace bb {

  std::unique_ptr<Command> mkCommand (DecodedCommand const & cmd)
  {
    switch (cmd.present)
    {
			case Command_PR_bb32wm: return std::unique_ptr<Command>(new Command_bb32wm(static_cast<unsigned>(cmd.choice.bb32wm.wmmsg)));
      default:
      {
        TRACE_MSG(LL_ERROR, CTX_BB | CTX_NET, "Unknown command");
        return nullptr;
      }
    }
  }

	struct Session : std::enable_shared_from_this<Session>
	{
    Server * m_server;
		asio::ip::tcp::socket m_socket;
		DecodingContext m_decodingCtx;
		Asn1Allocator m_asn1Allocator;

		Session (Server * s, asio::ip::tcp::socket socket)
			: m_server(s), m_socket(std::move(socket))
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

						DoReadHeader();
					}
					else
						m_socket.close();
				});
		}

		bool DoDecodeBody ()
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

			TryHandleCommand(m_decodingCtx.m_command);
			return true;
		}

		bool TryHandleCommand (DecodedCommand const & cmd)
		{
      std::unique_ptr<Command> c = mkCommand(cmd);
      if (c)
      {
        m_server->AddPendingRequest(std::move(c), this);
				return true;
      }
			return false;
		}
	};

	struct AsioServer
	{
		Server * m_server;
		asio::ip::tcp::acceptor m_acceptor;

		AsioServer (Server * s, asio::io_context & io, unsigned short port)
			: m_server(s)
			, m_acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
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
					std::make_shared<Session>(m_server, std::move(socket))->Start();
				}

				DoAccept();
			};
			m_acceptor.async_accept(fn);
		}
	};

	void Server::PostPendingResponses ()
	{
		asio::post(m_io, [this]() { m_}
// 		for (size_t i = 0, ie = m_responses.size(); i < ie; ++i)
// 		{
// 			std::unique_ptr<PendingCommand> resp = std::move(m_server.m_responses[i]);
// 			if (resp)
// 			{
// 				
// 			}
// 		}

	}

	bool Server::Run ()
	{
		try
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker started");
			asio::io_context io_context;
			AsioServer s(this, io_context, m_config.m_port);
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

