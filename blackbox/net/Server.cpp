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
		}

		void Start ()
		{
			HandleRead();
		}

		void HandleRead ()
		{
			//TRACE_SCOPE_MSG(LL_VERBOSE, CTX_BB | CTX_NET, "Server session @ 0x%x HandleRead");
			auto self(shared_from_this());
			
			char tmp[1024];
			m_socket.async_read_some(asio::buffer(tmp, 1024),
				[this, self, &tmp] (std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						OutputDebugStringA("data");
					}
				});
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

