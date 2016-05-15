#pragma once
#include <thread>
#include "ServerConfig.h"

namespace bb {

	struct Server
	{
		ServerConfig m_config;
		std::thread m_thread;

		Server () { }
		~Server () { }
		bool Init (ServerConfig const & cfg);
		bool Run ();
		bool Done ();
	};
}
