set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###############################################################################

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(buildbits 64)
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(buildbits 32)
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

###############################################################################

# this blocks ensures that no gcc .dlls are required for the run of an
# executable or plugin

# the pthread dependency is more difficult to handle, custom pthreads
# that are build as static library. then copied to the compiler tree
# sources for pthreads are here:
#		http://www.sourceware.org/pthreads-win32/
# or are already included in the tree:
#		3rd_party/pthreads-w32-2-9-1-release/
if (MINGW)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	add_definitions(-DPTW32_STATIC_LIB)
endif (MINGW)
if( ${CMAKE_COMPILER_IS_GNUCXX} )
	add_definitions(-static-libgcc)
	add_definitions(-static-libstdc++)
	set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libgcc -static-libstdc++ -lpthread")
	set(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -static-libgcc -static-libstdc++ -lpthread")
endif( ${CMAKE_COMPILER_IS_GNUCXX} )
# end of mingw 'no .dll dependency' stuff

###############################################################################

# there are lot of warnings from using unsecure functions
# FIXME: get rid of this in far future
if (MSVC)
	# there are lot of warnings from using unsecure functions
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	# these are mainly conversion bigger --> lesser int type
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")
endif (MSVC)


