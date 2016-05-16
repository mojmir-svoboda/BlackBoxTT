include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/bblib)
#include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

### shellhook
add_library(shellhook SHARED shellhook.cpp)
target_link_libraries(shellhook bblib)
target_link_libraries(shellhook version comctl32)
set_property(TARGET shellhook PROPERTY FOLDER "bbTT/hooks/shellhook")
install(TARGETS shellhook RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/shellhook.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)

### trayhook
add_library(trayhook SHARED trayhook.cpp)
target_link_libraries(trayhook bblib)
target_link_libraries(trayhook version comctl32)
set_property(TARGET trayhook PROPERTY FOLDER "bbTT/hooks/trayhook")
install(TARGETS trayhook RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/trayhook.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)


