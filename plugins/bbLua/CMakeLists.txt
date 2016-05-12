project(bbLua)
cmake_minimum_required(VERSION 2.8)

#add_executable(bbLua
#  WIN32
#
#  main.cpp
#  bblua.cpp
#  functions.cpp
#)

add_library(bbLua
  SHARED

  bbplugin.cpp
  bblua.cpp
  functions.cpp
)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
#include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set_property(TARGET bbLua PROPERTY FOLDER "plugins/bbLua")
target_link_libraries(bbLua version comctl32)

install(TARGETS bbLua RUNTIME DESTINATION plugins/bbLua)

