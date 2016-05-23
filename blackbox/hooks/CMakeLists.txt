include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/bblib)
#include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

### taskhook
add_library(taskhook SHARED taskhook.cpp)
target_link_libraries(taskhook bblib bbproto bbprotlib)
target_link_libraries(taskhook version comctl32)
set_property(TARGET taskhook PROPERTY FOLDER "bbTT/hooks/taskhook")
install(TARGETS taskhook RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/taskhook.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)

### trayhook
add_library(trayhook SHARED trayhook.cpp)
target_link_libraries(trayhook bblib bbprotolib)
target_link_libraries(trayhook version comctl32)
set_property(TARGET trayhook PROPERTY FOLDER "bbTT/hooks/trayhook")
install(TARGETS trayhook RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/trayhook.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)


