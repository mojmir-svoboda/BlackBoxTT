#pragma once
#include "Server.h"
#include <utility>
#include <memory>
#include <platform_win.h>
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <bbproto/decoder.h>
#include "SessionQueue.h"

namespace bb {

	struct Server;

	struct Session : std::enable_shared_from_this<Session>
	{
    Server * m_server;
		asio::io_context & m_io;
		asio::ip::tcp::socket m_socket;
		DecodingContext m_decodingCtx;
		Asn1Allocator m_asn1Allocator;
		asio::io_service::strand m_writeStrand;
		SpinLock m_responseLock;
		SessionQueue<std::unique_ptr<PendingCommand>> m_responses;

		Session (Server * s, asio::io_context & io, asio::ip::tcp::socket socket);
		void Start ();
		void DoReadHeader ();
		void DoReadBody ();
		bool DoDecodeBody ();
		bool QueueDecodedRequest (DecodedCommand const & cmd);
		bool AddResponse (std::unique_ptr<PendingCommand> cmd);
		void DoWriteResponse ();
	};


}

