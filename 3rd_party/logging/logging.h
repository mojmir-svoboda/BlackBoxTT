#pragma once

#define TRACE_ADAPT_LEVEL(x) (trace::level_t(1) << x)
#define TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC 1
#define TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE 1
#define TRACE_CONFIG_INCLUDE "config.h"
#define TRACE_LEVELS_INCLUDE "levels.h"
#define TRACE_CONTEXTS_INCLUDE "contexts.h"
#if !defined TRACE_STATIC
# define TRACE_STATIC
#endif

#if defined TRACE_ENABLED
#	include <3rd_party/DbgToolkit/trace_client/trace.h>
#else
#	include <3rd_party/logging/trace_dummy.h>
#endif
#include TRACE_LEVELS_INCLUDE
#include TRACE_CONTEXTS_INCLUDE

