#INSTALL_FILES = $(BIN) readme.txt
#INSTALL_IF_NEW = bbFoomp.rc

#add_definitions(-DUNICODE -D_UNICODE)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(bbFoomp_SOURCES
	src/bbfoomp.cpp
	src/foobar2000.cpp
	src/settings.cpp
	src/foompbutton.cpp
	src/styles.cpp
)

add_library(bbFoomp SHARED ${bbFoomp_SOURCES})
target_link_libraries(bbFoomp blackbox bblib bblibcompat)
target_link_libraries(bbFoomp version comctl32)
target_link_libraries(bbFoomp fnv)

set_property(TARGET bbFoomp PROPERTY FOLDER "plugins/bbFoomp")

#install(FILES readme.txt DESTINATION plugins/bbFoomp)
install(TARGETS bbFoomp RUNTIME DESTINATION plugins/bbFoomp)

if(MSVC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/bbFoomp.pdb DESTINATION plugins/bbFoomp  CONFIGURATIONS Debug)
endif(MSVC)
