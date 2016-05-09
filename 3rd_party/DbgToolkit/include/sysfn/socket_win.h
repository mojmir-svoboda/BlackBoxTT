#if defined __MINGW32__
#	undef _WIN32_WINNT
#	define _WIN32_WINNT 0x0600 
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <cstdlib>	// for atoi
//#include <cstdio>	// for vsnprintf etc
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#if defined WIN32
#	pragma comment (lib, "Ws2_32.lib")
#	pragma comment (lib, "Mswsock.lib")
#	pragma comment (lib, "AdvApi32.lib")
#elif defined WIN64
#	pragma comment (lib, "Ws2.lib")
#	pragma comment (lib, "Mswsock.lib")
#endif

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace sys { namespace socks {

	typedef SOCKET socket_t;

	inline bool is_connected (socket_t socket) { return socket != INVALID_SOCKET; }
	inline int get_errno () { return WSAGetLastError(); }
	inline bool is_timeouted () { return get_errno() == WSAETIMEDOUT; }

	inline void connect (char const * host, char const * port, socket_t & handle)
	{
		WSADATA wsaData;
		int const init_result = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (init_result != 0) {
			DBG_OUT("WSAStartup failed with error: %d\n", init_result);
			return;
		}

		addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo * result = NULL;
		int const resolve_result = getaddrinfo(host, port, &hints, &result); // resolve the server address and port
		if (resolve_result != 0) {
			DBG_OUT("getaddrinfo failed with error: %d\n", resolve_result);
			WSACleanup();
			return;
		}

		// attempt to connect to an address until one succeeded
		for (addrinfo * ptr = result; ptr != NULL ; ptr = ptr->ai_next)
		{
			// create a SOCKET for connecting to server
			socket_t new_handle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (new_handle == INVALID_SOCKET)
			{
				DBG_OUT("socket failed with error: %ld\n", get_errno());
				WSACleanup();
				return;
			}

			// Try connect to server...
			int const connect_result = connect(new_handle, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (connect_result == SOCKET_ERROR)
			{
				closesocket(new_handle);
				new_handle = INVALID_SOCKET;
				continue; // ... nope
			}

			handle = new_handle; // ...connected
			break;
		}

		freeaddrinfo(result);

		if (handle == INVALID_SOCKET)
		{
			DBG_OUT("Unable to connect to server!\n");
			WSACleanup();
			return;
		}

		int send_buff_sz = 64 * 1024;
		setsockopt(handle, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char *>(&send_buff_sz), sizeof(int));

		DWORD send_timeout_ms = 1;
		setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char *>(&send_timeout_ms), sizeof(DWORD));
	}

	inline void disconnect (socket_t & socket)
	{
		int const result = shutdown(socket, SD_SEND);
		if (result == SOCKET_ERROR)
			DBG_OUT("shutdown failed with error: %d\n", get_errno());
		closesocket(socket);
		socket = INVALID_SOCKET;
		WSACleanup();
	}
}}

