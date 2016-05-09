cmake_policy(SET CMP0048 NEW)
project (trace_proto VERSION 1.1.0)
cmake_minimum_required(VERSION 3.1)

# create targets
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/asn1c.cmake")
add_definitions("-DUSE_CXX_ALLOCATOR")

set(ASN1_SOURCES
  asn_codecs_prim.c
  asn_SEQUENCE_OF.c
  asn_SET_OF.c
  ber_decoder.c
  ber_tlv_length.c
  ber_tlv_tag.c
  BIT_STRING.c
  constr_CHOICE.c
  constr_SEQUENCE.c
  constr_SEQUENCE_OF.c
  constr_SET_OF.c
  constr_TYPE.c
  constraints.c
  der_encoder.c
  INTEGER.c
  NativeEnumerated.c
  NativeInteger.c
	NativeReal.c
  OCTET_STRING.c
	REAL.c
  BOOLEAN.c
  per_decoder.c
  per_encoder.c
  per_opentype.c
  per_support.c
  xer_decoder.c
  xer_encoder.c
  xer_support.c
)

set(ASN1_HEADERS
  asn_allocator.h
  asn_application.h
  asn_codecs.h
  asn_codecs_prim.h
  asn_SEQUENCE_OF.h
  asn_SET_OF.h
  asn_internal.h
  asn_system.h
  ber_decoder.h
  ber_tlv_length.h
  ber_tlv_tag.h
  BIT_STRING.h
  constr_CHOICE.h
  constr_SEQUENCE.h
  constr_SEQUENCE_OF.h
  constr_SET_OF.h
  constr_TYPE.h
  constraints.h
  der_encoder.h
  INTEGER.h
  NativeEnumerated.h
  NativeInteger.h
	NativeReal.h
  OCTET_STRING.h
	REAL.h
  BOOLEAN.h
  per_decoder.h
  per_encoder.h
  per_opentype.h
  per_support.h
  xer_decoder.h
  xer_encoder.h
  xer_support.h
)

set(TRACE_ASN1_HEADERS
  ConfigMsg.h
  Command.h
  DictPair.h
	DictionaryMsg.h
  LogMsg.h
  LogScopeType.h
	PlotMsg.h
  PlotMarkerMsg.h
  SoundMsg.h
)

set(TRACE_ASN1_SOURCES
  ConfigMsg.c
  Command.c
  DictionaryMsg.c
  DictPair.c
  LogMsg.c
  LogScopeType.c
  PlotMsg.c
  PlotMarkerMsg.c
  SoundMsg.c
)

set(GENERATED_SRC ${ASN1_SOURCES} ${TRACE_ASN1_SOURCES})

set(ASN1_PROTOCOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/trace_proto.asn1)

message("asn1c cmd: asn1c -gen-PER -S ${ASN1C_DATA} ${ASN1_PROTOCOL_FILE}")
message("*** GENERATOR TEMPORARILY TURNED OFF *** uncomment to re-generate and suffer")
#add_custom_command(
#  OUTPUT ${GENERATED_SRC}
#  COMMAND asn1c -gen-PER -S ${ASN1C_DATA} ${ASN1_PROTOCOL_FILE}
#	DEPENDS trace_proto.asn1
#)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(trace_proto ${GENERATED_SRC})
set_source_files_properties(${GENERATED_SRC} PROPERTIES LANGUAGE CXX)
set_target_properties(trace_proto PROPERTIES LINKER_LANGUAGE CXX)


# properties of targets
target_compile_definitions(trace_proto PUBLIC "trace_proto_DEBUG=$<CONFIG:Debug>")

set(config_install_dir "lib/${PROJECT_NAME}")
set(lib_install_dir "lib/${PROJECT_NAME}")
set(include_install_dir "include/${PROJECT_NAME}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
#set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file( "${version_config}" COMPATIBILITY SameMajorVersion)

# Configure '<prj>Config.cmake'
# Note: variable 'targets_export_name' used
configure_file("cmake/Config.cmake.in" "${project_config}" @ONLY)

install(TARGETS trace_proto 
  EXPORT "${targets_export_name}"
  LIBRARY DESTINATION "${lib_install_dir}"
  ARCHIVE DESTINATION "${lib_install_dir}"
  RUNTIME DESTINATION "bin"
  INCLUDES DESTINATION "${include_install_dir}"
)

# Headers:
function(prepend_path_and_install_asn args)
    set(list tmp)
    foreach(src ${args})
        install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/${src}"
            DESTINATION "${include_install_dir}/asn.1"
        )
    endforeach(src ${args})
endfunction(prepend_path_and_install_asn args)
prepend_path_and_install_asn("${ASN1_HEADERS}")
prepend_path_and_install_asn("${ASN1_SOURCES}")
prepend_path_and_install_asn("${TRACE_ASN1_HEADERS}")
prepend_path_and_install_asn("${TRACE_ASN1_SOURCES}")


set(TRACE_PROTO_INCLUDE_HEADERS
	dictionary.h
	header.h
	alloc.cpp
	alloc.h
	decoder.h
	decoder_alloc.h
	encode_config.h
	encode_dictionary.h
	encode_log.h
	encode_plot.h
	encoder.h
	trace_proto.asn1
	trace_proto.h
)

function(prepend_path_and_install args)
    set(list tmp)
    foreach(src ${args})
        install(
            FILES "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
            DESTINATION "${include_install_dir}"
        )
    endforeach(src ${args})
endfunction(prepend_path_and_install args)
prepend_path_and_install("${TRACE_PROTO_INCLUDE_HEADERS}")


# Config
#   * <prefix>/lib/cmake/<prj>/<prj>Config.cmake
#   * <prefix>/lib/cmake/<prj>/<prj>ConfigVersion.cmake
install(FILES "${project_config}" "${version_config}" DESTINATION "${config_install_dir}")

# Config
#   * <prefix>/lib/cmake/<prj>/<prj>Targets.cmake
install(EXPORT "${targets_export_name}" NAMESPACE "${namespace}" DESTINATION "${config_install_dir}")

