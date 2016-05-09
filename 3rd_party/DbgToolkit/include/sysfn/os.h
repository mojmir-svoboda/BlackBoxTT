#pragma once

#if defined WIN32 || defined WIN64 || defined _XBOX

#	if defined WIN32 || defined WIN64
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	elif defined _XBOX
#		include <xtl.h>
#		include <winbase.h>
#	endif
	namespace sys
	{
#	if defined WIN32 || defined WIN64
		inline unsigned get_pid () { return GetCurrentProcessId(); }
#	elif defined _XBOX
		inline unsigned get_pid () { return 0; }
#	endif
		inline unsigned get_tid () { return GetCurrentThreadId(); }

		/**@brief	yields core to other thread **/
		inline void thread_yield () { Sleep(1); }


		inline void delay_execution(unsigned & counter)
		{
			if (counter < 10)
				_mm_pause();
			else if (counter < 20)
				for (int i = 0; i < 50; ++i)
					_mm_pause();
			else if (counter < 22)
				SwitchToThread();
			else if (counter < 24)
				Sleep(0);
			else if (counter < 26)
				Sleep(1);
			else
				Sleep(10);

			counter += 1;
		}

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf sys::c99_snprintf
#define vsnprintf sys::c99_vsnprintf

		inline int c99_vsnprintf (char *outBuf, size_t size, const char * format, va_list ap)
		{
			int count = -1;

			if (size != 0)
				count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
			if (count == -1)
				count = _vscprintf(format, ap);

			return count;
		}

		inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
		{
			int count;
			va_list ap;

			va_start(ap, format);
			count = c99_vsnprintf(outBuf, size, format, ap);
			va_end(ap);

			return count;
		}

#endif

		/**@brief	simple encapsulation of CreateThread **/
		struct Thread
		{
			Thread (int prio) : m_handle(NULL), m_tid(), m_prio(prio) { }

			bool Create ( DWORD (WINAPI * fn) (void *), void * arg)
			{
				m_handle = CreateThread (
					0, // Security attributes
					0, // Stack size
					fn, arg,
					CREATE_SUSPENDED,
					&m_tid);
				if (m_handle)
					SetThreadPriority(m_handle, m_prio);

				return (m_handle != NULL);
			}

			~Thread () { }
			void Close () { if (m_handle) CloseHandle (m_handle); m_handle = 0; }
			void Resume () { if (m_handle) ResumeThread (m_handle); }
			void WaitForTerminate ()
			{
				if (m_handle)
					WaitForSingleObject(m_handle, 2000);
			}
		private:
			HANDLE m_handle;
			DWORD  m_tid;
			int    m_prio;
		};
	}
#else

#	include <sys/time.h>
#	include <stdint.h>
#	include <stdbool.h>
#	include <stddef.h>

	namespace sys {

		unsigned get_pid () { return getpid(); }
		unsigned get_tid () { return pthread_self(); } /// "hey piggy," i know

		inline tlv::len_t trc_vsnprintf (char * buff, size_t ln, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			int const n = vsnprintf(buff, ln, fmt, args);
			va_end(args);
			return static_cast<tlv::len_t>(n < 0 ? ln : n);
		}

		inline tlv::len_t va_trc_vsnprintf (char * buff, size_t ln, char const * fmt, va_list args)
		{
			int const n = vsnprintf(buff, ln, fmt, args);
			return static_cast<tlv::len_t>(n < 0 ? ln : n);
		}


		/**@brief	yields core to other thread **/
		inline void thread_yield () { pthread_yield(); }

		struct thread_info {    /* Used as argument to thread_start() */
			pthread_t thread_id;        /* ID returned by pthread_create() */
			int       thread_num;       /* Application-defined thread # */
			char     *argv_string;      /* From command-line argument */
		};

		/**@brief	simple encapsulation of CreateThread **/
#		define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#		define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
		struct Thread
		{
			pthread_attr_t m_attr;
			thread_info m_tinfo;

			Thread () { memset(this, 0, sizeof(*this)); }
			~Thread () { Close(); }

			void Resume () { }
			void Create (void * (* fn) (void *), void * )
			{
				/* Initialize thread creation attributes */
				int s = pthread_attr_init(&m_attr);
				if (s != 0)
					handle_error_en(s, "pthread_attr_init");
				/*if (stack_size > 0)
				{
					s = pthread_attr_setstacksize(&m_attr, stack_size);
					if (s != 0)
						handle_error_en(s, "pthread_attr_setstacksize");
				}*/

				m_tinfo.thread_num = 0;
				//m_tinfo.argv_string = argv[optind + t];

				/* The pthread_create() call stores the thread ID into corresponding element of m_tinfo */
				s = pthread_create(&m_tinfo.thread_id, &m_attr, fn, &m_tinfo);
				if (s != 0)
					handle_error_en(s, "pthread_create");
			}
			void WaitForTerminate ()
			{
				/* Destroy the thread attributes object, since it is no longer needed */
				int s = pthread_attr_destroy(&m_attr);
				if (s != 0)
					handle_error_en(s, "pthread_attr_destroy");

				void * res = 0;
				s = pthread_join(m_tinfo.thread_id, &res);
				if (s != 0)
				   handle_error_en(s, "pthread_join");

				printf("Joined with thread %d; returned value was %s\n", m_tinfo.thread_num, (char *) res);
				free(res);      /* Free memory allocated by thread */
			}
			void Close () { }
		};
#		undef handle_error_en
#		undef handle_error

	}

#endif


