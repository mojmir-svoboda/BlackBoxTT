#pragma once

namespace bb {

	struct ServerConfig
	{
		unsigned short m_port { 13199 };
		unsigned m_encodeBuffSz { 16384 };
	};
}
