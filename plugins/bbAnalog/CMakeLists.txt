set(bbAnalog_SOURCES
	bbAnalog.cpp
)

set(bbAnalog_CONFIGS bbAnalog.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbAnalog SHARED ${bbAnalog_SOURCES})
target_link_libraries(bbAnalog blackbox bblib bblibcompat fnv)
target_link_libraries(bbAnalog version comctl32)

set_property(TARGET bbAnalog PROPERTY FOLDER "plugins/bbAnalog")

install(FILES readme.txt DESTINATION plugins/bbAnalog)
install(TARGETS bbAnalog RUNTIME DESTINATION plugins/bbAnalog)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbAnalog/${bbAnalog_CONFIGS}")
	install(FILES ${bbAnalog_CONFIGS} DESTINATION plugins/bbAnalog)
endif()

if(MSVC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/bbAnalog.pdb DESTINATION plugins/bbAnalog  CONFIGURATIONS Debug)
endif(MSVC)

