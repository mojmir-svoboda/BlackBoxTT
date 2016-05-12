set(bbPager_SOURCES
  Drawing.cpp
  Settings.cpp
  bbPager.cpp
)

set(bbPager_CONFIGS bbpager.bb)
set(bbPager_RESOURCES bbPager_private.rc)
set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbPager SHARED ${bbPager_SOURCES} ${bbPager_RESOURCES})
target_link_libraries(bbPager blackbox bblib bbPlugin)
target_link_libraries(bbPager version comctl32)

set_property(TARGET bbPager PROPERTY FOLDER "plugins/bbPager")

#INSTALL_IF_NEW = bbpager.rc
install(FILES BBPager.html DESTINATION plugins/bbPager)
install(TARGETS bbPager RUNTIME DESTINATION plugins/bbPager)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbPager/${bbPager_CONFIGS}")
	install(FILES ${bbPager_CONFIGS} DESTINATION plugins/bbPager)
endif()

