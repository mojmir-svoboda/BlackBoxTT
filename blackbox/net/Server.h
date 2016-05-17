#pragma once
#include <thread>
#include "ServerConfig.h"
#include "Command.h"
#include <vector>
#include <bblib/SpinLock.h>

namespace bb {

	struct Server
	{
		ServerConfig m_config;
		std::thread m_thread;
		SpinLock m_lock;
    std::vector<Command *> m_queue;

		Server () : m_lock() { }
		~Server () { }
		bool Init (ServerConfig const & cfg);
		bool Run ();
		bool Done ();
		bool AddCommand (Command * c)
		{
			m_lock.Lock();
			m_queue.push_back(c);
			m_lock.Unlock();
			return true;
		}
	};
}

