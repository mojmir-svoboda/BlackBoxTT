#pragma once

#if defined	WIN32 || defined WIN64

#if defined TRACE_CLIENT_DISABLE_NETWORKING
# define TRACE_CLIENT_SINKS FileClient<typelist<LogISOTime, I, LogShortFile, Separator<':'>, LogLine, I, LogFunc, I>>
#else
# define TRACE_CLIENT_SINKS FileClient<typelist<LogISOTime, I, LogShortFile, Separator<':'>, LogLine, I, LogFunc, I>>, AsioSocketClient
#endif

#endif

