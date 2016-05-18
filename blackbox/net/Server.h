#pragma once
#include <thread>
#include "ServerConfig.h"
#include "Command.h"
#include <vector>
#include <bblib/SpinLock.h>

namespace bb {

	struct Session;
	struct PendingCommand
	{
		Session * m_session { nullptr };
		std::unique_ptr<Command> m_request;
		std::unique_ptr<Command> m_response;

		PendingCommand (std::unique_ptr<Command> cmd, Session * s) : m_session(s), m_request(std::move(cmd)) { }
	};

	struct Server
	{
		ServerConfig m_config;
		std::thread m_thread;
		SpinLock m_requestLock;
    std::vector<std::unique_ptr<PendingCommand>> m_requests;
		SpinLock m_responseLock;
    std::vector<std::unique_ptr<PendingCommand>> m_responses;

		Server () : m_requestLock(), m_responseLock() { m_requests.reserve(64); m_responses.reserve(64); }
		~Server () { }
		bool Init (ServerConfig const & cfg);
		bool Run ();
		bool Done ();
		bool AddPendingRequest (std::unique_ptr<Command> c, Session * s)
		{
			m_requestLock.Lock();

			std::unique_ptr<PendingCommand> p(new PendingCommand(std::move(c), s));
			m_requests.push_back(std::move(p));

			m_requestLock.Unlock();
			return true;
		}
		bool AddPendingResponse (std::unique_ptr<PendingCommand> p)
		{
			m_responseLock.Lock();

			m_responses.push_back(std::move(p));

			m_responseLock.Unlock();
			return true;
		}
	};
}

