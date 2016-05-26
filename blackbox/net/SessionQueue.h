#pragma once
#include <liblfds700.h>

namespace bb {

	template <class T>
	struct SessionQueue
	{
		lfds700_list_asu_state m_context;
		lfds700_ringbuffer_state m_ringbuff;
		lfds700_misc_prng_state m_prng;

		SessionQueue ()
		{
			lfds700_list_asu_init_valid_on_current_logical_core(&m_context, NULL, NULL);
			lfds700_misc_library_init_valid_on_current_logical_core();
			lfds700_misc_prng_init(&m_prng);
		}

		void Begin ()
		{
			LFDS700_MISC_BARRIER_LOAD;
		}

		bool Enqueue (T * t)
		{
			void * t0 = t;
			lfds700_queue_element * qe = qe = util_aligned_malloc(sizeof(struct lfds700_queue_element), LFDS700_PAL_ATOMIC_ISOLATION_IN_BYTES);
			int const rv = lfds700_queue_enqueue(&m_l, NULL, &m_prng);
			return rv == 1;
		}

		bool End ()
		{
			LFDS700_MISC_BARRIER_STORE;
			lfds700_misc_force_store();
		}

		bool Dequeue (T * & t)
		{
			lfds700_queue_element * qe = nullptr;
			int const rv = lfds700_queue_bss_dequeue(&m_queue, &qe, &m_prng);
			//t = static_cast<T *>(ptr);
			util_aligned_free(qe);
			return rv == 1;
		}

	};

}
