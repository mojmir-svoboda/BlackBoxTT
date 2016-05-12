include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/bblib)
#include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

add_library(shellhook SHARED shellhook.cpp)
target_link_libraries(shellhook bblib)
target_link_libraries(shellhook version comctl32)
set_property(TARGET shellhook PROPERTY FOLDER "hooks/shellhook")
install(TARGETS shellhook RUNTIME DESTINATION .)

add_library(trayhook SHARED trayhook.cpp)
target_link_libraries(trayhook bblib)
target_link_libraries(trayhook version comctl32)
set_property(TARGET trayhook PROPERTY FOLDER "hooks/trayhook")
install(TARGETS trayhook RUNTIME DESTINATION .)

