project(shellhook32on64)
cmake_minimum_required(VERSION 3.0)

add_definitions ("-DUNICODE")
add_definitions ("-D_UNICODE")
#FIXME: the paths are bit messed, this is a hotfix
include_directories(${CMAKE_SOURCE_DIR}/../../../blackbox)
include_directories(${CMAKE_SOURCE_DIR}/../../../bblib)

add_library(shellhook32 SHARED ../shellhook.cpp)
target_link_libraries(shellhook32 version comctl32)
#set_property(TARGET shellhook32 PROPERTY FOLDER "hooks/shellhook32")
install(TARGETS shellhook32 RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/shellhook32.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)

add_executable(blackbox32 WIN32 blackbox32.cpp)
target_link_libraries(blackbox32 shellhook32 version comctl32 shlwapi)
install(TARGETS blackbox32 RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/blackbox32.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)

###############################################################################
# debug stuff
#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
