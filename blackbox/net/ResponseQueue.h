#pragma once
#include <memory>

namespace boost { 
	namespace parameter { struct void_; }
	namespace lockfree {
		template <size_t Size>
		struct capacity;
#ifdef BOOST_NO_CXX11_VARIADIC_TEMPLATES
		template <typename T,
			class A0 = boost::parameter::void_,
			class A1 = boost::parameter::void_>
#else
		template <typename T, typename ...Options>
#endif
		class spsc_queue;
}}

namespace bb {
	struct ResponseQueue
	{
		//boost::lockfree::spsc_queue<std::unique_ptr<PendingCommand>, boost::lockfree::capacity<128>> * m_impl;
		boost::lockfree::spsc_queue<PendingCommand *, boost::lockfree::capacity<128>> * m_impl;

		ResponseQueue ();
		~ResponseQueue ();
		bool Enqueue (PendingCommand * cmd);
		bool Dequeue (PendingCommand * & cmd);
		//bool Enqueue(std::unique_ptr<PendingCommand> cmd);
		//bool Dequeue(std::unique_ptr<PendingCommand> & cmd);
	};

}
