project(bbSlit)

set(bbSlit_SOURCES
	bbSlit.cpp
)

set(bbSlit_CONFIGS bbSlit.rc)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbSlit SHARED ${bbSlit_SOURCES})
target_link_libraries(bbSlit blackbox bblib bblibcompat fnv)
target_link_libraries(bbSlit version comctl32)

set_property(TARGET bbSlit PROPERTY FOLDER "plugins/bbSlit")

install(FILES readme.txt DESTINATION plugins/bbSlit)
install(TARGETS bbSlit RUNTIME DESTINATION plugins/bbSlit)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbSlit/${bbSlit_CONFIGS}")
	install(FILES ${bbSlit_CONFIGS} DESTINATION plugins/bbSlit)
endif()

if(MSVC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/bbSlit.pdb DESTINATION plugins/bbSlit  CONFIGURATIONS Debug)
endif(MSVC)
