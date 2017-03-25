#pragma once
#include <atomic>

namespace bb {

	struct SpinLock
	{
		mutable std::atomic<bool> m_lock { false };

		void Lock () const
		{
			while (std::atomic_exchange_explicit(&m_lock, true, std::memory_order_acquire))
				;
		}

		void Unlock () const
		{
			std::atomic_store_explicit(&m_lock, false, std::memory_order_release);
		}
	};

}
