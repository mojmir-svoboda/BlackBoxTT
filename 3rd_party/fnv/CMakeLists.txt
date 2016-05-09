cmake_minimum_required (VERSION 2.8)
project (fnv)

add_library (fnv STATIC
    hash_32.cpp
    hash_32a.cpp
    hash_64.cpp
		hash_64a.cpp)

set_property(TARGET fnv PROPERTY FOLDER "3rd/fnv")

