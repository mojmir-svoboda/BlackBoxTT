
include_directories (${CMAKE_SOURCE_DIR}/3rd_party/asio/include)

add_definitions(-DASIO_STANDALONE)

set(net_SOURCES
  Server.cpp
)

set(net_HEADERS
  Server.h
)
add_library(net STATIC ${net_SOURCES} ${net_HEADERS})

set(bbcli_SOURCES
#  Client.cpp
	bbcli.cpp
)

set(bbcli_HEADERS
#  Client.h
)
add_executable(bbcli ${bbcli_SOURCES} ${bbcli_HEADERS})
install(TARGETS bbcli RUNTIME DESTINATION .	LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 

