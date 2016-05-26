#pragma once

namespace bb {

	enum class E_CommandType : unsigned;

	struct Command
	{
		virtual ~Command () { }
		virtual E_CommandType GetType () const = 0;
		virtual size_t Encode (char * buff, size_t buffsz) { return 0; }
	};
}
