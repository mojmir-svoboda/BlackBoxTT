project(bbSDK)
#add_definitions(-DUNICODE -D_UNICODE)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(bbSDK_SOURCES
  bbSDK.cpp
)

add_library(bbSDK SHARED ${bbSDK_SOURCES})
target_link_libraries(bbSDK blackbox bblib bblibcompat fnv)
target_link_libraries(bbSDK version comctl32)

set_property(TARGET bbSDK PROPERTY FOLDER "plugins/bbSDK")

install(FILES readme.txt DESTINATION plugins/bbSDK)
install(TARGETS bbSDK RUNTIME DESTINATION plugins/bbSDK)

