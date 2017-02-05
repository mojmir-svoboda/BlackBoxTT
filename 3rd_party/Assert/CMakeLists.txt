cmake_minimum_required (VERSION 3.0)
project (Assert)
include_directories(.)
add_library (Assert STATIC ppk_assert.h ppk_assert.cpp)
set_property(TARGET Assert PROPERTY FOLDER "3rd/Assert")

