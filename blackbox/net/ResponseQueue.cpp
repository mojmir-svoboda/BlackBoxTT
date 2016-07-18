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

  ResponseQueue::ResponseQueue ()
    : m_impl(new boost::lockfree::spsc_queue<PendingCommand *, boost::lockfree::capacity<128>>)
		//: m_impl(new boost::lockfree::spsc_queue<std::unique_ptr<PendingCommand>, boost::lockfree::capacity<128>>)
  {
  }

  ResponseQueue::~ResponseQueue ()
  {
  }

//   bool ResponseQueue::Enqueue (std::unique_ptr<PendingCommand> cmd)
//   {
//     return m_impl->push(cmd);
//   }
// 
//   bool ResponseQueue::Dequeue (std::unique_ptr<PendingCommand> & cmd)
//   {
//     return m_impl->pop(cmd);
//   }

	bool ResponseQueue::Enqueue(PendingCommand * cmd)
	{
		return m_impl->push(cmd);
	}

	bool ResponseQueue::Dequeue(PendingCommand * & cmd)
	{
		return m_impl->pop(cmd);
	}

}
