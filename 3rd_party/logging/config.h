#pragma once

#if defined	WIN32 || defined WIN64

//# define TRACE_CLIENT_DISABLE_NETWORKING 1
# define TRACE_CLIENT_SINKS AsioSocketClient, FileClient<typelist<LogISOTime, I, LogShortFile, Separator<':'>, LogLine, I, LogFunc, I>>

#endif

