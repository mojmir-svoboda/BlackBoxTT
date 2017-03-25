#include "trace.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <trace_proto/header.h>
#include <trace_proto/decoder.h>
#define NOMINMAX 1
#include <asio.hpp>
#include <sysfn/time_query.h>
#include <ScopeGuard.h>
#include "decoder.h"
#include "ClientConfig.h"

namespace trace {

	void OnConnectionEstablished ();
	void OnConnectionConfigCommand (Command const & cmd);

	struct SpinLock
	{
		std::atomic_flag m_lock { ATOMIC_FLAG_INIT };

		void Lock ()
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
		}
		
		void Unlock ()
		{
			m_lock.clear(std::memory_order_release);
		}
	};

	using buffptr_t = std::atomic<size_t>;
	struct MsgHeader
	{
		uint32_t m_size { 0 };
		uint32_t m_allocated { 0 };
	};

	struct ClientMemory
	{
		SpinLock m_lock;
		buffptr_t m_readPtr;
		buffptr_t m_writePtr;

		enum : size_t { e_memSize = 1024 * 1024 };
		enum : size_t { e_memAlign = 64 };
		alignas(e_memAlign) char m_memory[e_memSize];

		ClientMemory ()
		{
			//static_assert(alignof(m_readPtr) == 16, "problem");
			m_readPtr.store(0, std::memory_order_relaxed);
			m_writePtr.store(0, std::memory_order_relaxed);
		}

		char * AcquireMem (size_t n)
		{
			char * mem = nullptr;
			m_lock.Lock();

			size_t const curr_wr_ptr = m_writePtr; //m_writePtr.load(std::memory_order_acquire);
			size_t curr_space = e_memSize - curr_wr_ptr;
			void * const curr_mem = m_memory + curr_wr_ptr;
			void * next_mem = m_memory + curr_wr_ptr + n + sizeof(MsgHeader);
			if (std::align(e_memAlign, n, next_mem, curr_space))
			{
				mem = static_cast<char *>(curr_mem);
				char * const nmem = static_cast<char *>(next_mem);
				size_t const aligned_sz = nmem - mem;
				m_writePtr += aligned_sz;
				MsgHeader * hdr = new (mem) MsgHeader;
				hdr->m_allocated = aligned_sz;
			}

			m_lock.Unlock();
			return mem;
		}
	};
	extern ClientMemory g_ClientMemory;

	struct Client
	{
		SpinLock m_lock;
		ClientConfig m_config;
		bool m_terminated { false };
		bool m_connected { false };
		bool m_buffered { false };
		long long m_reconnect_ms { 3000 };
		unsigned m_reconnect_retry_count_max { 3 };
		unsigned m_reconnect_retry_count { 0 };
		std::thread m_thread;
		asio::io_context m_io;
		std::unique_ptr<asio::io_service::work> m_work;
		asio::ip::tcp::socket m_socket;
		asio::steady_timer m_timer;
		asio::ip::tcp::resolver::results_type m_endpoints;
		Decoder m_decoder;

		Client ()
			: m_timer(m_io)
			, m_socket(m_io)
		{
			DBG_OUT("Client created\n");
		}

		bool Init (char const * host, char const * port)
		{
			m_config.SetHostName(host);
			m_config.SetHostPort(port);

			try
			{
				asio::ip::tcp::resolver resolver(m_io);
				m_endpoints = resolver.resolve(host, port);
				if (bool const connected = DoConnect(m_endpoints))
				{
					OnConnected();
					const bool stopped = m_io.stopped();
					if (stopped)
						m_io.restart();
				}
				else
				{
					DBG_OUT("Cannot connect, will retry...\n");
					const bool stopped = m_io.stopped();
					if (stopped)
						m_io.restart();

					SetClientTimer(m_reconnect_ms);
					asio::async_connect(m_socket, m_endpoints,
							std::bind(&Client::OnConnect, this, std::placeholders::_1, m_endpoints));
					StartClientTimer();
				}

				m_work.reset(new asio::io_service::work(m_io));
				m_thread = std::thread(&Client::ThreadFunc, this);
				return true;
			}
			catch (std::exception & e)
			{
				DBG_OUT(e.what());
				return false;
			}
		}

		bool IsConnected () const { return m_connected; }
		void SetClientTimer (long long ms) // !MT
		{
			m_timer.expires_from_now(std::chrono::milliseconds(ms));
		}

		void CancelClientTimer () // !MT safe, T*
		{
			m_timer.cancel();
		}
		void StartClientTimer () // !MT safe, T*, Tio
		{
			if (m_terminated)
				return;
			DBG_OUT("StartClientTimer\n");
			m_timer.async_wait(std::bind(&Client::OnClientTimer, this, std::placeholders::_1));
		}

		void OnClientTimer (std::error_code const & error) // MT safe, Tio
		{
			if (error)
				return;

			scope_guard_t on_exit_unlock = mkScopeGuard(std::mem_fun(&SpinLock::Unlock), &m_lock);

			if (m_terminated)
				return;
			if (m_connected)
				return;

			if (m_timer.expires_at() <= std::chrono::steady_clock::now())
			{
				DBG_OUT("Reconnect expired\n");
				asio::error_code ignored_ec;
				m_socket.close(ignored_ec);

				StartReconnect();
			}

			StartClientTimer();
		}

		void StartReconnect () // !MT safe, T*
		{
			DBG_OUT("Reconnect started\n");

			m_connected = false;

			SetClientTimer(m_reconnect_ms);
			asio::async_connect(m_socket, m_endpoints,
					std::bind(&Client::OnConnect, this, std::placeholders::_1, m_endpoints));

			++m_reconnect_retry_count;
// 			if (m_reconnect_retry_count >= m_reconnect_retry_count_max)
// 			{
// 				DBG_OUT("Bad luck..\n");
// 			}
		}

		void OnConnected () // !MT safe, Tio
		{
			DBG_OUT("Connected!\n");
			m_connected = true;
			StartAsyncRead();
			OnConnectionEstablished();
		}

		void OnConnect (asio::error_code const & ec, asio::ip::tcp::resolver::results_type::iterator endpoint_iter) // ?MT safe, Tio
		{
			if (m_terminated)
				return;

			if (!m_socket.is_open())
			{
				DBG_OUT("OnConnect: socket not opened in time, reconnecting\n");
				StartReconnect();
			}
			else if (ec)
			{
				DBG_OUT("OnConnect: reconnecting because of error, code=%d msg=%s\n", ec.value(), ec.message().c_str());
				OnSocketError(ec);
				StartReconnect();
			}
			else
			{
				DBG_OUT("OnConnect: connected\n");
				CancelClientTimer();
				OnConnected();
			}
		}

		bool DoConnect (asio::ip::tcp::resolver::results_type const & endpoints) // Tmain
		{
			m_connected = false;
			asio::error_code result = asio::error::would_block;

			asio::async_connect(m_socket, endpoints,
				[&result](std::error_code ec, asio::ip::tcp::endpoint ep)
				{
					result = ec;
				});

			do
			{
				m_io.run_one();
			} while (result == asio::error::would_block); // sync wait for first connect

			if (result || !m_socket.is_open())
				return false;
			return true;
		}


		void ThreadFunc () // Tio
		{
			DBG_OUT("Started thread for io\n");
			m_io.run();
			DBG_OUT("Terminated thread for io\n");
		}

		void Done ()
		{
			Flush();
			m_terminated = true;
			m_work.reset();
			m_timer.cancel();
			m_socket.close();
			m_thread.join();
		}

		void Flush ()
		{
		}

		void OnAsyncWrite(std::error_code ec, std::size_t ln)
		{
			DBG_OUT("OnAsyncWrite sz=%i ec=0x%x\n", ln, ec);
			if (!ec)
			{
				AsyncWriteNext();
			}
			else
			{
				OnSocketError(ec);
				StartReconnect();
				StartClientTimer();
			}
		}

		void AsyncWriteNext ()
		{
			DBG_OUT("AsyncWriteNext\n");
			//m_pendingWrite.store(true, std::memory_order_release);

			g_ClientMemory.m_lock.Lock();
			size_t const rd_ptr = g_ClientMemory.m_readPtr;
			char const * const mem = g_ClientMemory.m_memory + rd_ptr;
			MsgHeader const * const hdr = reinterpret_cast<MsgHeader const *>(mem);
			asio::async_write(m_socket, asio::buffer(mem + sizeof(MsgHeader), hdr->m_size),
				std::bind(&Client::OnAsyncWrite, this, std::placeholders::_1, std::placeholders::_2));
			size_t const new_rd_ptr = rd_ptr + hdr->m_allocated;
			g_ClientMemory.m_writePtr = new_rd_ptr;
			g_ClientMemory.m_lock.Unlock();
		}


		std::error_code SyncWriteToSocket (char const * request, size_t ln) // MT safe (maybe)
		{
			std::error_code ec;
			asio::write(m_socket, asio::buffer(request, ln), ec); // should be MT safe on win32
			return ec;
		}

		void OnSocketError (std::error_code const & ec) // !MT safe, T*
		{
			DBG_OUT("OnSocketError: code=%d msg=%s\n", ec.value(), ec.message().c_str());
			m_connected = false;
			if (m_socket.is_open())
			{
				DBG_OUT("OnSocketError: closing socket.\n");
				m_socket.close();
			}
		}

		bool WriteToSocket (char const * request, size_t ln) { return Write(request, ln); } 
		bool Write (char const * buff, size_t ln) // ?MT safe, T*
		{
			if (IsConnected())
			{
				if (m_buffered)
				{
					char * buff_ptr = nullptr;
					if (WriteToBuffer(buff, ln, buff_ptr))
					{
// 						const bool has_pending = m_pendingWrite.load(std::memory_order_acquire);
// 						if (!has_pending)
// 							StartAsyncWrite();
					}
					else
					{
						// no room, spin/block
					}
				}
				else
				{
					std::error_code const ec = SyncWriteToSocket(buff, ln);
					if (ec)
					{
						scope_guard_t on_exit_unlock = mkScopeGuard(std::mem_fun(&SpinLock::Unlock), &m_lock);
						OnSocketError(ec);
						StartReconnect();
						StartClientTimer();
						return false;
					}
				}
			}
			else
			{
				char * buff_ptr = nullptr;
				if (!WriteToBuffer(buff, ln, buff_ptr))
				{
					return false;
				}
			}
			return true;
		}

		char * AcquireBufferMem (size_t ln)
		{
			char * mem = g_ClientMemory.AcquireMem(ln);
			return mem;
		}

// 		char * ReleaseBufferMem (char const * buff, size_t ln)
// 		{
// 		}

		bool WriteToBuffer (char const * msg, size_t ln, char * & buff_ptr)
		{
			char * mem = AcquireBufferMem(ln);
			if (mem)
			{
				memcpy(mem + sizeof(MsgHeader), msg, ln);
				MsgHeader * hdr = reinterpret_cast<MsgHeader *>(mem);
				hdr->m_size = ln;
				//ReleaseBufferMem(mem, ln);
				buff_ptr = mem + sizeof(MsgHeader);
				return true;
			}
			buff_ptr = nullptr;
			return false;
		}

		void OnConnectFlush ()
		{
			if (m_buffered)
			{
// 				const bool has_pending = m_pendingWrite.load(std::memory_order_acquire);
// 				if (!has_pending)
// 					AsyncWriteNext();
			}
			else
			{
				DBG_OUT("Flush on connect (sync)\n");
				g_ClientMemory.m_lock.Lock();

				while (g_ClientMemory.m_readPtr != g_ClientMemory.m_writePtr)
				{
					char const * const mem = g_ClientMemory.m_memory + g_ClientMemory.m_readPtr;
					MsgHeader const * const hdr = reinterpret_cast<MsgHeader const *>(mem);
					std::error_code ec;
					asio::write(m_socket, asio::buffer(mem + sizeof(MsgHeader), hdr->m_size), ec);
					if (ec)
					{
						OnSocketError(ec);
						StartReconnect();
						StartClientTimer();
					}
					else
					{
						g_ClientMemory.m_readPtr += hdr->m_allocated;
					}
				}
				g_ClientMemory.m_lock.Unlock();
			}
		}

		void Close ()
		{
			DBG_OUT("Cli::Close\n");
			asio::post(m_io, [this]() { m_socket.close(); });
		}

		void StartAsyncRead ()
		{
			DBG_OUT("Starting async read.\n");
			DoReadHeader ();
		}

		void DoReadHeader ()
		{
			asio::async_read(m_socket,
				asio::buffer(m_decoder.getEndPtr(), sizeof(asn1::Header)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec && length == sizeof(asn1::Header))
					{
						m_decoder.m_dcd_ctx.m_has_hdr = true;
						if (m_decoder.m_dcd_ctx.canMoveEnd(sizeof(asn1::Header)))
						{
							m_decoder.m_dcd_ctx.moveEnd(sizeof(asn1::Header));
							DoReadPayload();
						}
						else
						{
							// problem
						}
					}
					else
					{
						OnSocketError(ec);
						StartReconnect();
						StartClientTimer();
					}
				});
		}

		void DoReadPayload ()
		{
			DBG_OUT("Starting async read of payload.\n");
			size_t const payload_sz = m_decoder.m_dcd_ctx.getHeader().m_len;
			if (m_decoder.m_dcd_ctx.available() < payload_sz)
				OnSocketError(asio::error::no_buffer_space);

			asio::async_read(m_socket,
				asio::buffer(m_decoder.getEndPtr(), payload_sz),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec && length == m_decoder.m_dcd_ctx.getHeader().m_len)
					{
						if (m_decoder.m_dcd_ctx.canMoveEnd(length))
						{
							m_decoder.m_dcd_ctx.moveEnd(length);
							if (m_decoder.parseASN1())
							{
								if (m_decoder.m_dcd_ctx.m_command.present == Command_PR_config)
									OnConnectionConfigCommand(m_decoder.m_dcd_ctx.m_command);
							}
							else
							{
								DBG_OUT("Cannot decode received asn1 message.\n");
							}

							m_decoder.resetDecoder();
							StartAsyncRead();
						}
						else
						{
							// problem
						}
					}
					else
					{
						OnSocketError(ec);
						StartReconnect();
						StartClientTimer();
					}
				});
		}

		template<typename T>
		void SendDictionary (int type, T const * values, char const * names[], size_t dict_sz)
		{
			// send config message
			enum : size_t { max_msg_size = 8192 };
			char msg[max_msg_size];
			asn1::Header & hdr = asn1::encode_header(msg, max_msg_size);

			int64_t * const asn1_values = reinterpret_cast<int64_t *>(alloca(dict_sz * sizeof(int64_t)));
			for (size_t i = 0; i < dict_sz; ++i)
				asn1_values[i] = values[i];

			if (const size_t n = asn1::encode_dictionary(msg + sizeof(asn1::Header), max_msg_size - sizeof(asn1::Header), type, asn1_values, names, dict_sz))
			{
				hdr.m_len = n;
				SyncWriteToSocket(msg, n + sizeof(asn1::Header));
			}
		}

		// on connect (to server) callback
		void OnConnectionEstablished ()
		{
			OutputDebugStringA("LOG: connected, sending config to server\n");
			enum : size_t { max_msg_size = 1024 };
			char msg[max_msg_size];
			asn1::Header & hdr = asn1::encode_header(msg, max_msg_size);
			char const * mixer_ptr = reinterpret_cast<char const *>(m_config.m_mixer.data());
			size_t const mixer_sz = m_config.m_mixer.size() * sizeof(level_t);
			if (const size_t n = asn1::encode_config(msg + sizeof(asn1::Header), max_msg_size - sizeof(asn1::Header), m_config.m_appName, mixer_ptr, mixer_sz, m_config.m_buffered, sys::get_pid()))
			{
				hdr.m_len = n;
				SyncWriteToSocket(msg, n + sizeof(asn1::Header));
			}

			if (size_t n = m_config.m_levelValuesDict.size())
				SendDictionary(0, &m_config.m_levelValuesDict[0], &m_config.m_levelNamesDict[0], n);
			if (size_t n = m_config.m_contextValuesDict.size())
				SendDictionary(1, &m_config.m_contextValuesDict[0], &m_config.m_contextNamesDict[0], n);
		}

		// on config received (from server) callback
		void OnConnectionConfigCommand (Command const & cmd)
		{
			bool buffered = cmd.choice.config.buffered == 1;
			char grr[256];
			_snprintf_s(grr, 256, "LOG: received config command: buff=%u\n", buffered);
			OutputDebugString(grr);

/*			SetRuntimeBuffering(buffered);*/ // %%#$#$#&#@^#!^#@%^#%^@#%^@#%^@#%^

			OCTET_STRING const & omixer = cmd.choice.config.mixer;
			char const * const ptr = reinterpret_cast<char const *>(omixer.buf);
			level_t const * levels = reinterpret_cast<level_t const *>(ptr);
			assert(m_config.m_mixer.size() == omixer.size / sizeof(level_t));
			for (level_t & l : m_config.m_mixer)
				l = *levels++;
		}

	};
}
