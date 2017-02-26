project(blackbox)

include("${CMAKE_SOURCE_DIR}/build/version.cmake")

# version
add_custom_target(bbversion_h ALL)
add_custom_command(TARGET bbversion_h COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/build/version.cmake)
set_target_properties(bbversion_h PROPERTIES FOLDER "bbTT") 

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


# native hooks first
add_subdirectory(hooks)

# 32b for 64b build
Build32on64(taskhook hooks "32on64")
Build32on64(blackbox hooks "32on64") # NOTE: this is not blackbox, but blackbox32.exe

# bb networking
add_subdirectory(net)
set_target_properties(net PROPERTIES FOLDER "bbTT") 

# bb interpreter
add_subdirectory(scheme)

set(blackbox_SOURCES
	mainwndproc.cpp
	CommandLine.cpp
	BlackBox.cpp
	BlackBoxConfig.cpp
	BlackBox_binds.cpp
	BlackBox_broams.cpp
	BlackBox_compat.cpp
	Explorer.cpp
	gfx/MenuWidget.cpp
	VirtualDesktopManager.cpp
	DesktopWallpaper.cpp
	DesktopWallpaperConfig_yaml.cpp
	GfxConfig_yaml.cpp
	Tasks.cpp
	TasksConfig_yaml.cpp
	Tray.cpp
	TrayConfig_yaml.cpp #BlackBoxConfig_yaml.cpp
	WorkSpacesConfig_yaml.cpp
	WorkSpacesGraph.cpp
	WorkSpaces.cpp
	WidgetConfig_yaml.cpp
	bind/bind.cpp
	plugin/PluginManager.cpp
	plugin/PluginsConfig_yaml.cpp
	utils_recover.cpp
	utils_vdm.cpp
	menu/MenuConfig_yaml.cpp
)

set (blackbox_HEADERS
	common.h
	BlackBox.h
	BlackBox_compat.h
	BlackBoxConfig.h
	CommandLine.h
	Explorer.h
	VirtualDesktopManager.h
	DesktopWallpaper.h
	DesktopWallpaperConfig.h
	Tasks.h
	TaskConfig.h
	TasksConfig.h
	Tray.h
	TrayConfig.h
	WorkSpaces.h
	WidgetConfig.h
	utils_window.h
	utils_recover.h
	utils_yaml.h
	utils_vdm.h
	GfxConfig.h
	gfx/GuiWidget.h
	gfx/utils_widgets.h
	gfx/MenuWidget.h
	gfx/IconId.h
	gfx/GfxWindow.h
	gfx/utils_gdi.h
	plugin/plugin.h
	plugin/PluginManager.h
	plugin/PluginConfig.h
	plugin/PluginsConfig.h
	menu/Menuconfig.h
	menu/MenuConfig_yaml.h
)

set (blackbox_ImGui_SOURCES
	gfx/ImGui/DX11.cpp
	gfx/ImGui/Gfx.cpp
	gfx/ImGui/IconCache.cpp
	gfx/ImGui/GfxWindow.cpp
	gfx/ImGui/Gui.cpp
	#gfx/widgets/ImGui/PluginsWidget.cpp
	gfx/widgets/ImGui/StyleEditorWidget.cpp
	#gfx/widgets/ImGui/ControlPanelWidget.cpp
	#gfx/widgets/ImGui/DebugWidget.cpp
	#gfx/widgets/ImGui/TasksWidget.cpp
	#gfx/widgets/ImGui/MenuWidget.cpp
	#gfx/widgets/ImGui/RecoverWindowsWidget.cpp
	#gfx/widgets/ImGui/TrayWidget.cpp
	gfx/widgets/ImGui/PagerWidget.cpp
	#gfx/widgets/ImGui/QuickBarWidget.cpp
	gfx/widgets/ImGui/MenuWidget.cpp
	#gfx/widgets/ImGui/QuickBarWidget.cpp
)

set (blackbox_ImGui_HEADERS
	gfx/ImGui/imgui.h
	gfx/ImGui/utils_imgui.h
	gfx/ImGui/DX11.h
	gfx/ImGui/Gfx.h
	gfx/ImGui/GfxWindow.h
	gfx/ImGui/Gui.h
	gfx/ImGui/IconCache.h
	#gfx/widgets/ImGui/PluginsWidget.h
	gfx/widgets/ImGui/StyleEditorWidget.h
	#gfx/widgets/ImGui/ControlPanelWidget.h
	#gfx/widgets/ImGui/DebugWidget.h
	#gfx/widgets/ImGui/TasksWidget.h
	#gfx/widgets/ImGui/MenuWidget.h
	#gfx/widgets/ImGui/RecoverWindowsWidget.h
	#gfx/widgets/ImGui/TrayWidget.h
	gfx/widgets/ImGui/PagerWidget.h
	#gfx/widgets/ImGui/QuickBarWidget.h
	gfx/widgets/ImGui/MenuWidget.h
	#gfx/widgets/ImGui/QuickBarWidget.h
)

if (NOT MINGW)
	set(blackbox_SOURCES ${blackbox_SOURCES} ${PluginManager_SOURCES})
	set(blackbox_HEADERS ${blackbox_HEADERS} ${PluginManager_HEADERS})
endif (NOT MINGW)

#set(blackbox_RESOURCES resource.rc)
#set(CMAKE_RC_COMPILER_INIT windres)
#ENABLE_LANGUAGE(RC)


### blackbox dll

#add_library(bbImGui SHARED ${blackbox_ImGui_SOURCES} ${blackbox_ImGui_HEADERS})
add_library(bbImGui ${blackbox_ImGui_SOURCES} ${blackbox_ImGui_HEADERS})
target_link_libraries(bbImGui d3d11)
target_link_libraries(bbImGui d3dcompiler)
target_link_libraries(bbImGui imgui)
add_library(blackbox SHARED ${blackbox_SOURCES} ${blackbox_HEADERS} blackbox.def)
set_target_properties(blackbox PROPERTIES ENABLE_EXPORTS ON)
target_link_libraries(blackbox bblib)
target_link_libraries(blackbox version comctl32 shlwapi)
target_link_libraries(blackbox taskhook trayhook)
target_link_libraries(blackbox yaml-cpp)
target_link_libraries(blackbox dwmapi)
target_link_libraries(blackbox net)
target_link_libraries(blackbox scheme)
target_link_libraries(blackbox bbprotolib)
target_link_libraries(blackbox libcrazyrc)
target_link_libraries(blackbox Pathcch)
target_link_libraries(blackbox bbImGui)
target_link_libraries(blackbox Assert)
if(OPTION_USE_TRACE)
	target_link_libraries(blackbox ${TRACE_LIBRARY})
endif(OPTION_USE_TRACE)
#target_link_libraries(blackbox WICTextureLoader)
#target_link_libraries(blackbox WICTextureLoader runtimeobject)

install(TARGETS blackbox RUNTIME DESTINATION .	LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static) 
if(MSVC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/blackbox.pdb DESTINATION .	CONFIGURATIONS Debug)
endif(MSVC)

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
