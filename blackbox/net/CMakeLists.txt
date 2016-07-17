
include_directories(c:/devel/boost_1_61_0) # tmp @TODO @FIXME

set(net_SOURCES
  Server.cpp
  Session.cpp
	commands.cpp
  ResponseQueue.cpp
)

set(net_HEADERS
  Server.h
  Session.h
  ResponseQueue.h
	Command.h
	commands.h
)

add_library(net STATIC ${net_SOURCES} ${net_HEADERS})
target_link_libraries(net bbprotolib)

