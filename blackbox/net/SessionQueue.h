#pragma once
#include <liblfds700.h>

namespace bb {

	template <class T>
	struct Queue
	{
		lfds700_list_asu_state m_context;
		lfds700_ringbuffer_state m_ringbuff;

		Queue (size_t n)
		{
			lfds700_list_asu_init_valid_on_current_logical_core(&m_context, NULL, NULL);
			lfds700_misc_library_init_valid_on_current_logical_core();
		}

		void Begin ()
		{
			LFDS700_MISC_BARRIER_LOAD;
		}

		bool Enqueue (T * t)
		{
			void * t0 = t;
			int const rv = lfds700_queue_bss_enqueue(&m_l, NULL, t0);
			return rv == 1;
		}

		bool End ()
		{
			LFDS700_MISC_BARRIER_STORE;
			lfds700_misc_force_store();
		}

		bool Dequeue (T * & t)
		{
			//void * ptr = nullptr;
			//int const rv = lfds700_queue_bss_dequeue(&m_queue, NULL, &ptr);
			//t = static_cast<T *>(ptr);
			//return rv == 1;
		}

	};

}
