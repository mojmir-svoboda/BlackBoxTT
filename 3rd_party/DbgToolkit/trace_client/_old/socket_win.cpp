#include <sysfn/socket_win.h>
#include <sysfn/time_query.h>
#include "trace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <trace_proto/header.h>
#include <trace_proto/decoder.h>

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace trace {

	void OnConnectionEstablished ();
	void OnConnectionConfigCommand (Command const & cmd);

	namespace socks {

		using namespace sys::socks;

		std::atomic<bool> g_Quit(false);			/// request to quit
		std::atomic<bool> g_Flushed(false);			/// request to quit

		std::thread g_ThreadRecv;
		std::thread g_ThreadReconnect;
		socket_t g_Socket = INVALID_SOCKET;
		HANDLE g_LogFile = INVALID_HANDLE_VALUE;
		std::chrono::time_point<std::chrono::system_clock> g_ReconnectTimer;
		std::chrono::time_point<std::chrono::system_clock> g_ClottedTimer;
		std::chrono::milliseconds g_ReconnectTimerInterval(500);
		std::chrono::milliseconds g_ClottedTimerInterval(64);
		bool g_Clotted = false;
		std::function<void ()> g_onConnect;

		inline bool is_file_connected () { return g_LogFile != INVALID_HANDLE_VALUE; }

		inline bool WriteToFile (char const * buff, size_t ln)
		{
			if (is_file_connected())
			{
				DWORD written;
				WriteFile(g_LogFile, buff, static_cast<DWORD>(ln), &written, 0);
				return true;
			}
			return false;
		}

		void schedule_reconnect ()
		{
			socks::g_ReconnectTimer = std::chrono::system_clock::now() + g_ReconnectTimerInterval;
		}

		bool WriteToSocket (char const * buff, size_t ln)
		{
			if (sys::socks::is_connected(socks::g_Socket))
			{
				int result = SOCKET_ERROR;
				if (!g_Clotted)
					result = send(g_Socket, buff, (int)ln, 0);
				else
				{
					if (std::chrono::system_clock::now() >= g_ClottedTimer)
					{
						result = send(g_Socket, buff, (int)ln, 0);
						if (result != SOCKET_ERROR && result > 0)
						{
							DBG_OUT("declotted\n");
							g_Clotted = false;
						}
					}
					else
					{
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
						WriteToFile(buff, ln);
#	ifdef TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC
						OutputDebugStringA("dropped some log messages!");
#	endif
#endif
						return true;
					}
				}

				bool const timeouted = ( result == SOCKET_ERROR && is_timeouted());
				if (result == SOCKET_ERROR && !timeouted)
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					closesocket(g_Socket);
					WSACleanup();
					g_Socket = INVALID_SOCKET;

#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
					WriteToFile(buff, ln);
#	ifdef TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC
					OutputDebugStringA("dropping log messages!");
#	endif
#endif
					return false;
				}
				else if (timeouted || result == 0)
				{
					DBG_OUT("socked clotted\n");
					g_Clotted = true;
					g_ClottedTimer = std::chrono::system_clock::now() + g_ClottedTimerInterval;
					return true;
				}
			}
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
			else
			{
				WriteToFile(buff, ln);
#	ifdef TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC
				OutputDebugStringA("dropping log messages!");
#	endif
			}
#endif
			DBG_OUT(".");
			return true;
		}

		/**@brief	function receiving commands from server **/
		void receive_thread ( )
		{
			Asn1Allocator allocator;
			allocator.resizeStorage(allocator.calcNextSize());
			size_t const hdr_sz = sizeof(asn1::Header);
			DecodingContext dcd_ctx;
			while (!g_Quit)
			{
				size_t batch_size = 0;

				while (1)
				{
					// @TODO @TODO @TODO fix this !!!! partial receive needed as in connection_stream.cpp
					if (!dcd_ctx.m_has_hdr)
					{
						char * const buff_ptr = dcd_ctx.getEndPtr();
						int const count = recv(g_Socket, buff_ptr, static_cast<int>(hdr_sz), 0); // read header
						if (count <= 0)
							break;	 // not enough data
						else
						{
							dcd_ctx.moveEndPtr(count);

							if (dcd_ctx.getSize() == hdr_sz)
								dcd_ctx.m_has_hdr = true; // message header ready
							else
								break;  // not enough data
						}
					}
					else
					{
						if (!dcd_ctx.m_has_payload)
						{
							char * const buff_ptr = dcd_ctx.getEndPtr();
							asn1::Header const & hdr = dcd_ctx.getHeader();
							// @TODO @FIXME hdr.m_len is wrong, correct is len - already_received
							int const count = recv(g_Socket, buff_ptr, hdr.m_len, 0); // read payload
							if (count <= 0)
								break;  // not enough data
							else
							{
								dcd_ctx.moveEndPtr(count);

								if (dcd_ctx.getSize() == hdr_sz + hdr.m_len)
									dcd_ctx.m_has_payload = true; // message payload ready
							}
						}

						if (dcd_ctx.m_has_payload)
						{
							if (dcd_ctx.getHeader().m_version == 1)
							{
								Command * cmd_ptr = &dcd_ctx.m_command;
								void * cmd_void_ptr = cmd_ptr;
								char const * payload = dcd_ctx.getPayload();
								asn1::Header const & hdr = dcd_ctx.getHeader();
								size_t const av = allocator.available();
								size_t const size_estimate = hdr.m_len * 4; // at most
								if (av < size_estimate)
								{	// not enough memory for asn1 decoder
									assert(0);
								}

								const asn_dec_rval_t rval = ber_decode(&allocator, 0, &asn_DEF_Command, &cmd_void_ptr, payload, hdr.m_len);
								if (rval.code != RC_OK)
								{
									dcd_ctx.resetCurrentCommand();
									allocator.Reset();
									assert(0);
									break;
								}

								OnConnectionConfigCommand(dcd_ctx.m_command);

								dcd_ctx.resetCurrentCommand(); // reset current decoder command for another decoding pass
							}
						}
					}
				}

				dcd_ctx.resetCurrentCommand();
				allocator.Reset();
			}
			return;
		}

		bool try_connect ();
		void Flush () { }
	}

	namespace socks {

		bool try_connect (char const * hostname, char const * port)
		{
			socks::connect(hostname, port, g_Socket);

			if (socks::is_connected(socks::g_Socket))
			{
				socks::g_ThreadRecv = std::thread(socks::receive_thread);
				OnConnectionEstablished();
				return true;
			}
			else
			{
				return false;
			}
		}

		void reconnect_proc ()
		{
			for (;;)
			{
				if (try_connect(GetHostName(), GetHostPort()))
				{
					return;
				}
				else
				{
					std::this_thread::sleep_for(g_ReconnectTimerInterval);
				}
			}
		}
	}

	void Connect (char const * host, char const * port)
	{
		sys::setTimeStart();
		SetHostName(host);
		SetHostPort(port);

		bool const connected = socks::try_connect(host, port);
		if (!connected)
		{
			socks::g_ThreadReconnect = std::thread(socks::reconnect_proc);
		}

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		//char filename[128];
		//create_log_filename(filename, sizeof(filename) / sizeof(*filename));
		//socks::g_LogFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		//msg_t msg;
		// send cmd_setup message
		//encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());
		//socks::WriteToFile(msg.m_data, msg.m_length);
#	endif

		//socks::g_ThreadSend.Resume();
	}

	bool WriteToSocket (char const * buff, size_t ln)
	{
		return socks::WriteToSocket(buff, ln);
	}

	void Disconnect ()
	{
		Flush();
		socks::g_Quit = true;
		socks::g_ThreadReconnect.join();
		socks::g_ThreadRecv.join();

		if (socks::is_connected(socks::g_Socket))
			sys::socks::disconnect(socks::g_Socket);

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		CloseHandle(socks::g_LogFile);
#	endif
	}

	void Flush () { socks::Flush(); }
}

