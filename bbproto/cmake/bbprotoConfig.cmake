include("${CMAKE_CURRENT_LIST_DIR}/bbprotoTargets.cmake")

get_filename_component(bbproto_INCLUDE_DIRS "${SELF_DIR}/../../include/bbproto" ABSOLUTE)
set(bbproto_LIBRARIES bbproto)
