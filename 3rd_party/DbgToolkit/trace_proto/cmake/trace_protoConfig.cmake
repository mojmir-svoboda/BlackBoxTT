include("${CMAKE_CURRENT_LIST_DIR}/trace_protoTargets.cmake")

get_filename_component(trace_proto_INCLUDE_DIRS "${SELF_DIR}/../../include/trace_proto" ABSOLUTE)
set(trace_proto_LIBRARIES trace_proto)
