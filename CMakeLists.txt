project(BlackBoxTT)
message("Welcome to BlackBox!\n")
cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_VERBOSE_MAKEFILE on)

###############################################################################################################################
### OPTIONS
option(OPTION_DEVEL "Development build" OFF)
option(OPTION_RUN_ASN1C "Generate code from asn.1 (run asn1c)" OFF)
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

	#get_filename_component(UTF_BASE_NAME ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE} NAME_WE)
	#get_filename_component(UTF_PATH ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE} PATH)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/Release/bin/logging.dll DESTINATION .
		CONFIGURATIONS Release RelWithDebInfo
		)

	#get_filename_component(UTF_BASE_NAME_DEBUG ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG} NAME_WE)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/Debug/bin/logging.dll
		${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/logging/Debug/bin/logging.pdb
		DESTINATION .
		CONFIGURATIONS Debug
		)

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
include_directories(3rd_party/liblfds7.0.0/liblfds700/inc)
# asio
add_definitions("-DASIO_STANDALONE")
include_directories (${CMAKE_SOURCE_DIR}/3rd_party/asio/include)

include(ExternalProject)
# TODO directory into arguments
function(Build32on64 proj_name proj_dir proj_subdir)
	if ( buildbits EQUAL 64 )
		set(${proj_name}_SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/${proj_dir})
		set(${proj_name}32_PATH ${${proj_name}_SOURCE_DIR}/${proj_subdir})
		message("32on64: ${proj_name} sources=${${proj_name}32_PATH}")
		ExternalProject_Add (
			${proj_name}32
			CMAKE_GENERATOR ${GENERATOR_FOR_32}
      CMAKE_ARGS -DBB32on64=1 -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
      BINARY_DIR ${CMAKE_BINARY_DIR}/${proj_name}32
			SOURCE_DIR	${${proj_name}32_PATH}
			DOWNLOAD_COMMAND ""
		)
		set_property(TARGET ${proj_name}32 PROPERTY FOLDER "${proj_dir}/${proj_name}")
	endif( buildbits EQUAL 64 )
endfunction()

# bbproto + asn1
add_definitions("-DASSUMESTDTYPES") # asn1: use types from cstdint 
add_definitions("-DUSE_CXX_ALLOCATOR")
if (OPTION_RUN_ASN1C)
include_directories(${CMAKE_BINARY_DIR}/bbproto)
else (OPTION_RUN_ASN1C)
  include_directories(${CMAKE_SOURCE_DIR}/bbproto/generated)
endif (OPTION_RUN_ASN1C)
# rc reader
add_subdirectory(crazyrc)
set_target_properties(libcrazyrc PROPERTIES FOLDER "bbTT") 
# blackbox core
add_subdirectory(bblib)
set_target_properties(bblib PROPERTIES FOLDER "bbTT") 
add_subdirectory(bbproto)
Build32on64(bbproto bbproto "")
set_target_properties(bbprotolib PROPERTIES FOLDER "bbTT") 
add_subdirectory(blackbox)
set_target_properties(blackbox PROPERTIES FOLDER "bbTT") 
# plugins
add_subdirectory(bblibcompat)
set_target_properties(bblibcompat PROPERTIES FOLDER "bbTT") 
add_subdirectory(plugins)
# tools
add_subdirectory(tools)

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
