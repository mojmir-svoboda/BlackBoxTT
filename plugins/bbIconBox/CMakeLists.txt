set(bbIconBox_SOURCES
	bbIconBox.cpp
	LoadFolder.cpp
	DropTarget.cpp
	Utils.cpp
	winlist.cpp
)

set(bbIconBox_CONFIGS bbIconBox.rc)
set(bbIconBox_RESOURCES resource.rc)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(bbIconBox SHARED ${bbIconBox_SOURCES} ${bbIconBox_RESOURCES})
target_link_libraries(bbIconBox blackbox bblib bbPlugin)
target_link_libraries(bbIconBox version comctl32)

set_property(TARGET bbIconBox PROPERTY FOLDER "plugins/bbIconBox")

install(FILES readme.txt DESTINATION plugins/bbIconBox)
install(TARGETS bbIconBox RUNTIME DESTINATION plugins/bbIconBox)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbIconBox/${bbIconBox_CONFIGS}")
	install(FILES ${bbIconBox_CONFIGS} DESTINATION plugins/bbIconBox)
endif()

