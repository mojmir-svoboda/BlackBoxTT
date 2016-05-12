project(bblibcompat)
add_definitions(-DUNICODE -D_UNICODE)

set(bblibcompat_SOURCES
  bblibcompat.cpp
  bbImage.cpp
	bbrc.cpp
	styleprops.cpp
	colors.cpp
	winutils.cpp
	bbPlugin.cpp
	bbSettings.cpp
)

set(bblibcompat_HEADERS
  bblibcompat.h
  bbImage.h
)

add_library(bblibcompat STATIC ${bblibcompat_SOURCES})

target_link_libraries(bblibcompat version comctl32)
#target_link_libraries(bblibcompat)
