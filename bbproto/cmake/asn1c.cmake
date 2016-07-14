#find_program(ASN1C asn1c)
#if(NOT ASN1C)
  #message("Warning: asn1c not in system path!")
#endif()

if (WIN32)
	set(ASN1C_PATH ${CMAKE_SOURCE_DIR}/3rd_party/asn1c)

  message("asn1c searching in: ${ASN1C_PATH}")

  find_program(ASN1C_EXECUTABLE
    NAMES "asn1c.exe"
		HINTS "${ASN1C_PATH}/bin"
    DOC "asn1c x86 command line client")

	exec_program(${ASN1C_EXECUTABLE} ${ASN1C_PATH} ARGS "-h" OUTPUT_VARIABLE tmpvar)
	string(REGEX MATCH "^ASN.1 Compiler, (v[0-9.]*)" tmpvarver "${tmpvar}")
  message("asn1c version: ${tmpvarver}")
	unset(tmpvar)
	unset(tmpvarver)
endif (WIN32)

add_executable(asn1c IMPORTED)
set_property(TARGET asn1c PROPERTY IMPORTED_LOCATION ${ASN1C_EXECUTABLE})

set(ASN1C_DATA "${CMAKE_SOURCE_DIR}/3rd_party/asn1c/skeletons")
