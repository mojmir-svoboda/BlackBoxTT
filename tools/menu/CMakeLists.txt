project(bbmenu)

set(bbmenu_SOURCES bbmenu.cpp CommandLine.cpp)
set(bbmenu_HEADERS CommandLine.h)

add_executable(bbmenu WIN32 ${bbmenu_SOURCES} ${bbmenu_HEADERS})
target_link_libraries(bbmenu bbprotolib)
target_link_libraries(bbmenu Assert)
set_target_properties(bbmenu PROPERTIES FOLDER "tools") 
install(TARGETS bbmenu RUNTIME DESTINATION .	LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 

