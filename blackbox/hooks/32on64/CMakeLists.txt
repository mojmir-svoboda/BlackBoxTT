project(taskhook32on64)
cmake_minimum_required(VERSION 3.0)

add_definitions ("-DUNICODE")
add_definitions ("-D_UNICODE")
#FIXME: the paths are bit messed, this is a hotfix
include_directories(${CMAKE_SOURCE_DIR}/../../../blackbox)
include_directories(${CMAKE_SOURCE_DIR}/../../../bblib)

include("${CMAKE_SOURCE_DIR}/../../../build/FindReleaseAndDebug.cmake")
FIND_RELEASE_AND_DEBUG(BBPROTO32 bbprotolib32.lib bbprotolib32.lib ${CMAKE_CURRENT_BINARY_DIR}/../bbproto32/Release ${CMAKE_CURRENT_BINARY_DIR}/../bbproto32/Debug)
message("taskhook32 using bbproto32: ${BBPROTO32}")
message("taskhook32 using bbproto32: ${BBPROTO32_LIBRARY}")

add_library(taskhook32 SHARED ../taskhook.cpp ${files})
target_link_libraries(taskhook32 version comctl32 ${BBPROTO32_LIBRARY})

#set_property(TARGET taskhook32 PROPERTY FOLDER "bbTT/hooks")
install(TARGETS taskhook32 RUNTIME DESTINATION .)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/taskhook32.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)

add_executable(blackbox32 WIN32 blackbox32.cpp)
target_link_libraries(blackbox32 taskhook32 version comctl32 shlwapi)
#set_property(TARGET blackbox32 PROPERTY FOLDER "bbTT/hooks")
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
