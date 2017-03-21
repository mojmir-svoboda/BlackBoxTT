#include "Server.h"
#include <blackbox/common.h>
#include <utility>
#include <memory>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
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
				TRACE_MSG(LL_VERBOSE, CTX_BB | CTX_NET, "Server worker got connection");
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
		, m_io(nullptr)
	{
		m_requests.reserve(64);
		m_responses.reserve(64);
	}
	Server::~Server ()
	{ }
	
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
					m_encodingBuffer.clear();
					if (size_t const n = resp->m_response->Encode(&m_encodingBuffer[0], m_encodingBuffer.size()))
					{
						p->AddResponse(std::move(resp));
					}
				}
			}
		}
	}

	bool Server::Run ()
	{
		try
		{
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server worker started");
			AsioServer s(this, *m_io, m_config.m_port);
			m_io->run();
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
		m_encodingBuffer.resize(m_config.m_encodeBuffSz);
		TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "Server init, port=%u", cfg.m_port);
		m_io.reset(new asio::io_context);
		m_thread = std::thread(&Server::Run, this);
		return true;
	}

	bool Server::Done ()
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server terminating...");
		// @TODO: terminate asio io ctx
		if (m_io)
		{
			m_io->stop();
			m_thread.join();
			m_io.reset();
		}
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_NET, "Server terminated.");
		return true;
	}
}

