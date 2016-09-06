project(bbcli)

set(bbcli_SOURCES bbcli.cpp CommandLine.cpp)
set(bbcli_HEADERS CommandLine.h)

add_executable(bbcli WIN32 ${bbcli_SOURCES} ${bbcli_HEADERS})
target_link_libraries(bbcli bbprotolib)
set_target_properties(bbcli PROPERTIES FOLDER "tools") 
install(TARGETS bbcli RUNTIME DESTINATION .	LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 

