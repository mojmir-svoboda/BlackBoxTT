project(BlackBox)
message("Welcome to BlackBox!")
cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_VERBOSE_MAKEFILE on)

###############################################################################################################################
### OPTIONS
option(OPTION_DEVEL "Development build" OFF)
option(OPTION_USE_TRACE "Logging" OFF)
option(OPTION_USE_TCL "TCL" OFF)

# use this for develoment, if you do not pass it from command line:
# development build means:
#  - logging enabled
#  - overriden install path
#  - not overwriting config .rc files (for most annoying ones so far)
#set(OPTION_DEVEL 1)

if (OPTION_DEVEL)
	set(CMAKE_INSTALL_PREFIX "c:/bb_devel")
else(OPTION_DEVEL)
endif(OPTION_DEVEL)

# possibly enable c++11 for gcc
#if (MINGW)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
#endif (MINGW)

include_directories (.)
include_directories (bblib)

### find libraries
include("${CMAKE_CURRENT_SOURCE_DIR}/build/FindReleaseAndDebug.cmake")

if (OPTION_USE_TRACE)
	add_definitions ("-DTRACE_ENABLED")
	FIND_RELEASE_AND_DEBUG(TRACE logging.lib logging.lib ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/Release ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/Debug)
endif (OPTION_USE_TRACE)

message("bb generator=${CMAKE_GENERATOR}")
message("bb install path=${CMAKE_INSTALL_PREFIX}")

include(build/compiler.cmake)
include(build/install_if_absent.cmake)
#include(ExternalProject)

# detect 32 / 64bit environment
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(buildbits 64)
	message("bb build=64bit")
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(buildbits 32)
	message("bb build=32bit")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

add_definitions ("-DUNICODE")
add_definitions ("-D_UNICODE")

add_subdirectory(3rd_party)
include_directories (3rd_party)
include_directories (3rd_party/yaml-cpp/include)


# rc reader
add_subdirectory(crazyrc)
# blackbox core
add_subdirectory(bblib)
add_subdirectory(bbproto)
add_subdirectory(blackbox)
# plugins
add_subdirectory(bblibcompat)
add_subdirectory(plugins)

# installation
set(main_CONFIGS
	cfg/blackbox.yaml
)

if(OPTION_DEVEL)
	# install config files
	#install_if_absent(. ${main_CONFIGS})
	# install styles etc
#  install(DIRECTORY styles backgrounds DESTINATION .)
endif(OPTION_DEVEL)


###############################################################################
# debug stuff
#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#		message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
