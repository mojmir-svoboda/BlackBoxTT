cmake_minimum_required (VERSION 2.8)
project (crazyrc)

#set(Boost_DEBUG 1)
#FIND_PACKAGE(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  add_definitions("-DHAS_BOOST")
else (Boost_FOUND)

  message("crazyrc requires boost::spirit. that's THE reason for the 'crazy' part in the name...")

	# hotfix for boost with no libraries
	include_directories(c:/devel/boost_1_59_0) # tmp @TODO @FIXME

endif (Boost_FOUND)

add_library (libcrazyrc SHARED
    rc_parser.cpp
    rc_generate.cpp
    rc_cache.cpp
    rc_index.cpp
    rc.cpp
    rgb_txt_parser.cpp
    )
target_link_libraries(libcrazyrc Shlwapi)

install(TARGETS libcrazyrc RUNTIME DESTINATION .  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 

if(MSVC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/libcrazyrc.pdb DESTINATION .  CONFIGURATIONS Debug)
endif(MSVC)

#add_executable (testcrazyrc main.cpp)
#target_link_libraries(testcrazyrc libcrazyrc Shlwapi)

