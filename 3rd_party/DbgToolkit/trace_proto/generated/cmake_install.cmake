# Install script for directory: C:/devel/DbgToolkit/trace_proto

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "c:/opt/")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/Debug/trace_proto.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/Release/trace_proto.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/MinSizeRel/trace_proto.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/RelWithDebInfo/trace_proto.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_allocator.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_application.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_codecs.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_codecs_prim.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_SEQUENCE_OF.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_SET_OF.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_internal.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_system.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ber_decoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ber_tlv_length.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ber_tlv_tag.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/BIT_STRING.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_CHOICE.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_SEQUENCE.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_SEQUENCE_OF.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_SET_OF.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_TYPE.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constraints.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/der_encoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/INTEGER.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/NativeEnumerated.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/NativeInteger.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/NativeReal.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/OCTET_STRING.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/REAL.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/BOOLEAN.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_decoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_encoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_opentype.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_support.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/xer_decoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/xer_encoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/xer_support.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_codecs_prim.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_SEQUENCE_OF.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/asn_SET_OF.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ber_decoder.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ber_tlv_length.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ber_tlv_tag.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/BIT_STRING.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_CHOICE.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_SEQUENCE.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_SEQUENCE_OF.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_SET_OF.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constr_TYPE.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/constraints.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/der_encoder.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/INTEGER.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/NativeEnumerated.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/NativeInteger.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/NativeReal.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/OCTET_STRING.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/REAL.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/BOOLEAN.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_decoder.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_encoder.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_opentype.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/per_support.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/xer_decoder.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/xer_encoder.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/xer_support.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ConfigMsg.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/Command.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/DictPair.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/DictionaryMsg.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/LogMsg.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/LogScopeType.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/PlotMsg.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/PlotMarkerMsg.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/SoundMsg.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/ConfigMsg.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/Command.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/DictionaryMsg.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/DictPair.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/LogMsg.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/LogScopeType.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/PlotMsg.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/PlotMarkerMsg.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto/asn.1" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/SoundMsg.c")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/dictionary.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/header.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/alloc.cpp")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/alloc.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/decoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/decoder_alloc.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/encode_config.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/encode_dictionary.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/encode_log.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/encode_plot.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/encoder.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/trace_proto.asn1")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/trace_proto/trace_proto.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE FILE FILES
    "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/generated/trace_protoConfig.cmake"
    "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/generated/trace_protoConfigVersion.cmake"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/trace_proto/trace_protoTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/trace_proto/trace_protoTargets.cmake"
         "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/CMakeFiles/Export/lib/trace_proto/trace_protoTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/trace_proto/trace_protoTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/trace_proto/trace_protoTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/CMakeFiles/Export/lib/trace_proto/trace_protoTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/CMakeFiles/Export/lib/trace_proto/trace_protoTargets-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/CMakeFiles/Export/lib/trace_proto/trace_protoTargets-minsizerel.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/CMakeFiles/Export/lib/trace_proto/trace_protoTargets-relwithdebinfo.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/trace_proto" TYPE FILE FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_proto/CMakeFiles/Export/lib/trace_proto/trace_protoTargets-release.cmake")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/Debug/trace_proto.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/Release/trace_proto.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/MinSizeRel/trace_proto.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE STATIC_LIBRARY FILES "C:/devel/DbgToolkit/_projects_vs14_x64/trace_server/Bin/RelWithDebInfo/trace_proto.lib")
  endif()
endif()

