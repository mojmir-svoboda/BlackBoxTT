#pragma once

#if defined WIN32 || defined WIN64 || defined _XBOX

#	if defined WIN32 || defined WIN64
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	elif defined _XBOX
#		include <xtl.h>
#	endif

	namespace sys
	{
		typedef LONGLONG hptimer_t;

		extern hptimer_t g_Start, g_Freq;
		//hptimer_t g_Start, g_Freq;

		inline hptimer_t queryPerformanceFrequency ()
		{
			LARGE_INTEGER tmp;
			QueryPerformanceFrequency(&tmp);
			hptimer_t const res = tmp.QuadPart;
			return res;
		}

		inline hptimer_t queryPerformanceCounter ()
		{
			LARGE_INTEGER tmp;
			QueryPerformanceCounter(&tmp);
			hptimer_t const res = tmp.QuadPart;
			return res;
		}

		inline void setTimeStart ()
		{
			if (g_Start == 0)
			{
				g_Freq = queryPerformanceFrequency();
				g_Start = queryPerformanceCounter();
			}
		}
		inline hptimer_t queryTime () { return queryPerformanceCounter() - g_Start; }
		inline hptimer_t queryTime_ms () { return 1000 * (queryPerformanceCounter() - g_Start) / g_Freq; } // @TODO: get rid of div
		inline hptimer_t queryTime_us () { return 1000000 * (queryPerformanceCounter() - g_Start) / g_Freq; }
		inline double toSeconds (hptimer_t t) { return static_cast<double>(t) / g_Freq; }
	}
#else

#	include <sys/time.h>
#	include <stdint.h>
#	include <stdbool.h>
#	include <stddef.h>

	namespace sys
	{
		static const unsigned usec_per_sec = 1000000;
		static const unsigned usec_per_msec = 1000;

		typedef int64_t hptimer_t;
		extern hptimer_t g_Start, g_Freq;

		inline hptimer_t queryPerformanceFrequency ()
		{
			 return usec_per_sec;		/* gettimeofday reports to microsecond accuracy. */
		}

		inline hptimer_t queryPerformanceCounter ()
		{
			 struct timeval time;
			 gettimeofday(&time, NULL);
			 hptimer_t performance_count = time.tv_usec + time.tv_sec * usec_per_sec; /* Seconds. */
			 return performance_count;
		}

		inline void setTimeStart ()
		{
			if (g_Start == 0)
			{
				g_Freq = queryPerformanceFrequency();
				g_Start = queryPerformanceCounter();
			}
		}

		inline hptimer_t queryTime () { return queryPerformanceCounter() - g_Start; }
		inline hptimer_t queryTime_us () { return queryTime(); }
		inline hptimer_t queryTime_ms () { return queryTime_us() / 1000; }
		inline double toSeconds (hptimer_t t) { return static_cast<double>(t) / g_Freq; }
	}

#endif

namespace sys {

	struct Timer
	{
		hptimer_t m_expire_at;

		Timer () : m_expire_at(0) { }
		void set_delay_ms (unsigned delay_ms) { m_expire_at = queryTime_ms() + delay_ms; }
		void reset () { m_expire_at = 0; }
		bool enabled () const { return m_expire_at != 0; }
		bool expired () const { return queryTime_ms() > m_expire_at; }
	};
}

