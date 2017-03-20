
#set(Boost_DEBUG 1)
FIND_PACKAGE(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  add_definitions("-DHAS_BOOST")
endif (Boost_FOUND)

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

