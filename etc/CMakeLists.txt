project(copy_configs)

file(GLOB ConfigFiles
	${CMAKE_SOURCE_DIR}/etc/*.yaml
  ${CMAKE_SOURCE_DIR}/etc/*.ahk)

add_custom_target(copy_configs)
foreach(ConfigFile ${ConfigFiles})
	INSTALL(FILES ${ConfigFile} DESTINATION etc)
endforeach()
