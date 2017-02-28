add_definitions(-DUNICODE -D_UNICODE)

if (OPTION_USE_TRACE)
	set(bblib_trace_HEADERS
		${CMAKE_SOURCE_DIR}/3rd_party/logging/config.h
		${CMAKE_SOURCE_DIR}/3rd_party/logging/logging.h
		${CMAKE_SOURCE_DIR}/3rd_party/logging/levels.h
		${CMAKE_SOURCE_DIR}/3rd_party/logging/contexts.h
	)
endif (OPTION_USE_TRACE)

set(bblib_SOURCES
  utils.cpp
	logging.cpp
)

set(bblib_HEADERS
	bblib.h
  bbregex.h
  bbstring.h
  codecvt.h
  Color.h
  logging.h
  platform_win.h
  SpinLock.h
  unicode.h
  utils_file.h
	wcslcpy.h
	${bblib_trace_HEADERS}
)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)

add_library(bblib STATIC ${bblib_SOURCES} ${bblib_HEADERS})

target_link_libraries(bblib version comctl32)
target_link_libraries(bblib)
