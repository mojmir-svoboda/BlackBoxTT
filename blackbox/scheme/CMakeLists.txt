
set(scheme_SOURCES
  Scheme.cpp
  #SchemeConfig.cpp
)

set(scheme_HEADERS
  Scheme.h
  SchemeConfig.h
)

add_library(scheme STATIC ${scheme_SOURCES} ${scheme_HEADERS})
target_link_libraries(scheme s7)

