project(blackbox)

include("${CMAKE_SOURCE_DIR}/build/version.cmake")

# version
add_custom_target(bbversion_h ALL)
add_custom_command(TARGET bbversion_h COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/build/version.cmake)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)

if ( buildbits EQUAL 64 )
	if (MSVC)
		string(REGEX REPLACE " Win64" "" GENERATOR_FOR_32 ${CMAKE_GENERATOR})
	endif (MSVC)
	if (MINGW)
		# TODO
	endif (MINGW)
	message("${arg} 32 on 64bit... 32bit generator=${GENERATOR_FOR_32}")
endif( buildbits EQUAL 64 )


include(ExternalProject)
# TODO directory into arguments
function(Build32on64 proj_name proj_dir)
	if ( buildbits EQUAL 64 )
		set(${proj_name}_SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/${proj_dir})
		set(${proj_name}32_PATH ${${proj_name}_SOURCE_DIR}/32on64)
		message("${proj_name} 32 on 64bit... path=${${proj_name}32_PATH}")
		ExternalProject_Add (
			${proj_name}32
			CMAKE_GENERATOR ${GENERATOR_FOR_32}
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			SOURCE_DIR	${${proj_name}32_PATH}
			DOWNLOAD_COMMAND ""
		)
		set_property(TARGET ${proj_name}32 PROPERTY FOLDER "${proj_dir}/${proj_name}")
	endif( buildbits EQUAL 64 )
endfunction()

# native hooks first
add_subdirectory(hooks)

# 32b for 64b build
Build32on64(shellhook hooks)
Build32on64(blackbox hooks) # NOTE: this is not blackbox, but blackbox32.exe


set(blackbox_SOURCES
	mainwndproc.cpp
	CommandLine.cpp
	BlackBox.cpp
	BlackBoxConfig.cpp
	BlackBox_binds.cpp
	Explorer.cpp
	Tasks.cpp
	TasksConfig_yaml.cpp
	Tray.cpp
	TrayConfig_yaml.cpp #BlackBoxConfig_yaml.cpp
	#WorkSpacesConfig_yaml.cpp
	gfx/DX11.cpp
	gfx/Gfx.cpp
	gfx/GfxWindow.cpp
	gfx/IconCache.cpp
	gfx/Gui.cpp
	widgets/Plugins.cpp
	widgets/StyleEditor.cpp
	widgets/ControlPanel.cpp
	widgets/Debug.cpp
	widgets/Tasks.cpp
	widgets/Tray.cpp
	Menu/Menu.cpp
	bind/bind.cpp
	plugin/PluginManager.cpp
	plugin/PluginsConfig_yaml.cpp
)

set (blackbox_HEADERS
	BlackBox.h
	BlackBoxConfig.h
	CommandLine.h
	Explorer.h
	Tasks.h
	TaskConfig.h
	TasksConfig.h
	Tray.h
	TrayConfig.h
	utils_window.h
	utils_yaml.h
	gfx/DX11.h
	gfx/Gfx.h
	gfx/GfxWindow.h
	gfx/Gui.h
	gfx/GuiWidget.h
	gfx/IconCache.h
	gfx/IconId.h
	gfx/imgui.h
	gfx/utils_gdi.h
	gfx/utils_imgui.h
	widgets/Plugins.h
	widgets/StyleEditor.h
	widgets/ControlPanel.h
	widgets/Debug.cpp
	widgets/Tasks.cpp
	widgets/Tray.cpp
	plugin/plugin.h
	plugin/PluginManager.h
	plugin/PluginConfig.h
	plugin/PluginsConfig.h
)

if (NOT MINGW)
	set(blackbox_SOURCES ${blackbox_SOURCES} ${PluginManager_SOURCES})
	set(blackbox_HEADERS ${blackbox_HEADERS} ${PluginManager_HEADERS})
endif (NOT MINGW)

#set(blackbox_RESOURCES resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)

#if(MINGW)
# SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
#endif(MINGW)

add_library(blackbox SHARED ${blackbox_SOURCES} ${blackbox_HEADERS} blackbox.def)
set_target_properties(blackbox PROPERTIES ENABLE_EXPORTS ON)
target_link_libraries(blackbox bblib)
target_link_libraries(blackbox version comctl32 shlwapi)
target_link_libraries(blackbox shellhook trayhook)
target_link_libraries(blackbox yaml-cpp)
target_link_libraries(blackbox imgui)
target_link_libraries(blackbox dwmapi)
target_link_libraries(blackbox d3d11)
target_link_libraries(blackbox d3dcompiler)
if(OPTION_USE_TRACE)
	target_link_libraries(blackbox ${TRACE_LIBRARY})
endif(OPTION_USE_TRACE)
#target_link_libraries(blackbox WICTextureLoader)
#target_link_libraries(blackbox WICTextureLoader runtimeobject)
install(TARGETS blackbox RUNTIME DESTINATION .	LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/blackbox.pdb DESTINATION .	CONFIGURATIONS Debug)
	#install(TARGETS logging DESTINATION .)
endif(MSVC)

# libcrazyrc
target_link_libraries(blackbox libcrazyrc Shlwapi)
#install(TARGETS libcrazyrc RUNTIME DESTINATION .  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 
#if(MSVC)
#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/libcrazyrc.pdb DESTINATION .  CONFIGURATIONS Debug)
#endif(MSVC)

### main exe

add_executable(main WIN32 main.cpp ${blackbox_RESOURCES})
target_link_libraries(main blackbox)
install(TARGETS main RUNTIME DESTINATION .	LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static)
#set_target_properties(main PROPERTIES OUTPUT_NAME blackbox)
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/main.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)



### script bindings

#find_package(SWIG REQUIRED)
#include(UseSWIG)

#find_package(PythonLibs)
#include_directories(${PYTHON_INCLUDE_PATH})

#set(CMAKE_SWIG_FLAGS "")

#set_source_files_properties(bind/bind.swg PROPERTIES CPLUSPLUS ON)
#set_source_files_properties(bind/bind.swg PROPERTIES SWIG_FLAGS "-includeall")
#swig_add_module(bind_lua lua bind/bind.swg bind/bind.cpp)
#swig_link_libraries(bind_lua blackbox)
