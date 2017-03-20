#pragma once

#if defined	WIN32 || defined WIN64

# define TRACE_CLIENT_DISABLE_NETWORKING 1
# define TRACE_CLIENT_SINKS FileClient
# define TRACE_FILE_CLIENT_FORMAT LogTime, I, LogLevel, I, LogContext, I, LogFile, Separator<':'>, LogLine, I, LogFunc, I
//#	define TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE	1

#endif

