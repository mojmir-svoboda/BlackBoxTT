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
		std::weak_ptr<Session> m_session;
		std::unique_ptr<Command> m_request;
		std::unique_ptr<Command> m_response;
		std::vector<char> m_encodedResponse;

		PendingCommand (std::unique_ptr<Command> cmd, std::shared_ptr<Session> s) : m_session(s), m_request(std::move(cmd)) { }
	};

	struct AsioServer;
	struct Server
	{
		ServerConfig m_config;
		std::thread m_thread;
		SpinLock m_requestLock;
    std::vector<std::unique_ptr<PendingCommand>> m_requests;
    std::vector<std::unique_ptr<PendingCommand>> m_responses;
		std::unique_ptr<AsioServer> m_impl;
		std::vector<std::weak_ptr<Session>> m_sessions;
		std::vector<char> m_encodingBuffer;

		Server ();
		~Server ();
		bool Init (ServerConfig const & cfg);
		bool Run ();
		bool Done ();
		bool AddRequest (std::unique_ptr<Command> c, std::shared_ptr<Session> s);
		void DispatchResponses ();
	};
}

