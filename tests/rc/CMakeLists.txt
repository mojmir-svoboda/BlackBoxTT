project(BlackboxPi)
message("Welcome to BlackBox Pi!")
cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_VERBOSE_MAKEFILE on)

# use this for develoment, if you do not pass it from command line:
# development build means:
#  - logging enabled
#  - overriden install path
#  - not overwriting config .rc files (for most annoying ones so far)
set(DEVEL_BUILD 1)

if(DEVEL_BUILD EQUAL 1)
  set(CMAKE_INSTALL_PREFIX "c:/bbPi_devel")
  #set(USE_LOGGING 1)
  set(USE_LOGGING 0)
else(DEVEL_BUILD EQUAL 1)
  set(DEVEL_BUILD 0)
endif(DEVEL_BUILD EQUAL 1)

# possibly enable c++11 for gcc
#if (MINGW)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
#endif (MINGW)

#include_directories (3rd_party/fnv)
include_directories (.)

if(USE_LOGGING EQUAL 1)
  include_directories (3rd_party/DbgToolkit)
  include_directories (3rd_party/logging)
	add_definitions ("-DTRACE_ENABLED")
	add_definitions ("-DTRACE_STATIC")
	add_definitions ("-DTRACE_CONFIG_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/config.h\"")
	add_definitions ("-DTRACE_LEVELS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/levels.h\"")
	add_definitions ("-DTRACE_CONTEXTS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/contexts.h\"")
else(USE_LOGGING EQUAL 1)
endif(USE_LOGGING EQUAL 1)

message("bb generator=${CMAKE_GENERATOR}")
message("bb install path=${CMAKE_INSTALL_PREFIX}")

include(build/compiler.cmake)
include(build/install_if_absent.cmake)
include(ExternalProject)

# detect 32 / 64bit environment
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(buildbits 64)
  message("bb build=64bit")
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(buildbits 32)
  message("bb build=32bit")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

add_definitions(-DUNICODE -D_UNICODE)

set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS 1)
FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )

    #set(BOOST_LIBS filesystem system)
    #    find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
    #ENDIF()
endif (Boost_FOUND)

add_subdirectory(3rd_party)
add_subdirectory(blackbox)
#add_subdirectory(blackbox/Hooks)

# installation
set(main_CONFIGS
  config/menu.rc 
  config/rgb.txt 
  config/blackbox.rc
)

if(DEVEL_BUILD EQUAL 0)
  # install config files
  install_if_absent(. ${main_CONFIGS})
  # install styles etc
  install(DIRECTORY styles backgrounds DESTINATION .)
endif(DEVEL_BUILD EQUAL 0)

###############################################################################
# debug stuff
#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#   message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
