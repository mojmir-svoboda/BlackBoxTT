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
#include "Session.h"

namespace bb {

	struct AsioServer
	{
		Server * m_server;
		asio::ip::tcp::acceptor m_acceptor;
		asio::io_context & m_io;

		AsioServer (Server * s, asio::io_context & io, unsigned short port)
			: m_server(s)
			, m_acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			, m_io(io)
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
					std::make_shared<Session>(m_server, m_io, std::move(socket))->Start();
				}

				DoAccept();
			};
			m_acceptor.async_accept(fn);
		}
	};

	Server::Server ()
		: m_requestLock()
		, m_impl(nullptr)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server @ 0x%x", this);
		m_requests.reserve(64);
		m_responses.reserve(64);
	}
	Server::~Server ()
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "~Server @ 0x%x", this);
	}
	
	bool Server::AddRequest (std::unique_ptr<Command> c, std::shared_ptr<Session> s)
	{
		m_requestLock.Lock();

		std::unique_ptr<PendingCommand> p(new PendingCommand(std::move(c), s));
		m_requests.push_back(std::move(p));

		m_requestLock.Unlock();
		return true;
	}


	void Server::DispatchResponses ()
	{
		for (size_t i = 0, ie = m_responses.size(); i < ie; ++i)
		{
			std::unique_ptr<PendingCommand> resp = std::move(m_responses[i]);
			if (resp)
			{
				if (std::shared_ptr<Session> p = resp->m_session.lock())
				{
					p->AddResponse(std::move(resp));
				}
			}
		}
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

