#pragma once

#define TRACE_ARGUMENT_ORDER ArgLevel, ArgContext, ArgFile, ArgLine, ArgFunc

namespace trace {

	struct ArgLevel
	{
		static const constexpr bool c_requires_arg { true };
		using arg_t = level_t;
	};

	struct ArgContext
	{
		static const constexpr bool c_requires_arg{ true };
		using arg_t = context_t;
	};

	struct ArgFile
	{
		static const constexpr bool c_requires_arg { true };
		using arg_t = char const *;
	};

	struct ArgLine
	{
		static const constexpr bool c_requires_arg { true };
		using arg_t = int;
	};

	struct ArgFunc
	{
		static const constexpr bool c_requires_arg { true };
		using arg_t = char const *;
	};
}

