#pragma once
#include "trace_arguments.h"
#include <ctime>
#include <process.h>
#include <thread>

namespace trace {

	// writers with arguments

	struct LogLevel : ArgLevel
	{
		size_t Write (char * buff, size_t n, arg_t arg) const
		{
			size_t const written = snprintf(buff, n, "%x", arg);
			return written;
		}
	};

	struct LogContext : ArgContext
	{
		size_t Write (char * buff, size_t n, arg_t arg) const
		{
			size_t const written = snprintf(buff, n, "%x", arg);
			return written;
		}
	};

	struct LogFile : ArgFile
	{
		size_t Write (char * buff, size_t n, arg_t arg) const
		{
			size_t const written = snprintf(buff, n, "%s", arg);
			return written;
		}
	};
	struct LogShortFile : ArgFile
	{
		size_t Write (char * buff, size_t n, arg_t arg) const
		{
			char const * slash = strrchr(arg, '/');
			if (slash == nullptr)
				slash = strrchr(arg, '\\'); // backslash.. what an atrocity!
			size_t const written = snprintf(buff, n, "%s", slash ? slash + 1 : arg);
			return written;
		}
	};

	struct LogLine : ArgLine
	{
		size_t Write (char * buff, size_t n, arg_t arg) const
		{
			size_t const written = snprintf(buff, n, "%i", arg);
			return written;
		}
	};

	struct LogFunc : ArgFunc
	{
		size_t Write (char * buff, size_t n, arg_t arg) const
		{
			size_t const written = snprintf(buff, n, "%s", arg);
			return written;
		}
	};

	struct LogMsg
	{
		size_t Write (char * buff, size_t buffsz, char const * fmt, va_list args) const
		{
			int const n = vsnprintf(buff,  buffsz, fmt, args);
			if (n > 0 && n < buffsz)
				return n;
			return 0;
		}
	};

	/// misc writers

	struct LogTime
	{
		static const constexpr bool c_requires_arg { false };
		size_t Write (char * buff, size_t n/*, void*/) const
		{
			std::time_t const t = std::time(NULL);
			return std::strftime(buff, n, "%A %c", std::localtime(&t));
		}
	};
	struct LogStdTime
	{
		static const constexpr bool c_requires_arg { false };
		size_t Write(char * buff, size_t n/*, void*/) const
		{
			std::time_t const t = std::time(NULL);
			return std::strftime(buff, n, "%c", std::localtime(&t));
		}
	};
	struct LogISOTime
	{
		static const constexpr bool c_requires_arg { false };
		size_t Write(char * buff, size_t n/*, void*/) const
		{
			std::time_t const t = std::time(NULL);
			return std::strftime(buff, n, "%F %T", std::localtime(&t));
		}
	};


	template<char C>
	struct Separator
	{
		static const constexpr bool c_requires_arg { false };
		static const constexpr char c_separator { C };

		size_t Write (char * buff, size_t n/*, void*/) const
		{
			if (n > 0)
			{
				buff[0] = c_separator;
				return 1;
			}
			return 0;
		}
	};

	using I = Separator<'|'>;

	struct LogPID
	{
		static const constexpr bool c_requires_arg{ false };
		size_t Write (char * buff, size_t n/*, void*/) const
		{
			size_t const written = snprintf(buff, n, "%i", ::_getpid());
			return written;
		}
	};

	struct LogTID
	{
		static const constexpr bool c_requires_arg{ false };
		size_t Write (char * buff, size_t n/*, void*/) const
		{
			size_t const written = snprintf(buff, n, "%i", std::this_thread::get_id());
			return written;
		}
	};

}

