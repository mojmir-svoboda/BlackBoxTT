
set(net_SOURCES
  Server.cpp
  Session.cpp
	commands.cpp
)

set(net_HEADERS
  Server.h
  Session.h
	Command.h
	commands.h
)

add_library(net STATIC ${net_SOURCES} ${net_HEADERS})
target_link_libraries(net bbprotolib)

