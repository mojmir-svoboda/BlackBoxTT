
set(bbInterface_SOURCES
	./src/AgentMaster.cpp
	./src/AgentType_AutoScaleGraph.cpp
	./src/AgentType_Bitmap.cpp
	./src/AgentType_Broam.cpp
	./src/AgentType_Clock.cpp
	./src/AgentType_CompoundText.cpp
	./src/AgentType_DiskSpaceMonitor.cpp
	./src/AgentType_Graph.cpp
	./src/AgentType_iTunes.cpp
	./src/AgentType_Mixer.cpp
	./src/AgentType_Mixer_XP.cpp
	./src/AgentType_Mixer_Vista.cpp
	./src/AgentType_NetworkMonitor.cpp
	./src/AgentType_Run.cpp
	./src/AgentType_StaticText.cpp
	./src/AgentType_SwitchedState.cpp
	./src/AgentType_System.cpp
	./src/AgentType_SystemInfo.cpp
	./src/AgentType_SystemMonitor.cpp
	./src/AgentType_TGA.cpp
	./src/AgentType_Winamp.cpp
	./src/ColorMaster.cpp
	./src/ConfigMaster.cpp
	./src/ControlMaster.cpp
	./src/ControlType_Button.cpp
	./src/ControlType_Label.cpp
	./src/ControlType_Slider.cpp
	./src/DialogMaster.cpp
	./src/ListMaster.cpp
	./src/MenuMaster.cpp
	./src/MessageMaster.cpp
	./src/ModuleMaster.cpp
	./src/PluginMaster.cpp
	./src/SlitManager.cpp
	./src/SnapWindow.cpp
	./src/StyleMaster.cpp
	./src/Tooltip.cpp
	./src/WindowMaster.cpp
	./src/utils/core_load.cpp
)

set(bbInterface_RESOURCES bbInterface.rc)

#add_definitions(-DUNICODE)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
include_directories(${CMAKE_SOURCE_DIR}/3rd_party/iTunesCOMWindowsSDK)
add_library(bbInterface SHARED ${bbInterface_SOURCES})
target_link_libraries(bbInterface blackbox bblib bbPlugin)
target_link_libraries(bbInterface version gdiplus comsuppw iphlpapi comctl32 winmm)
target_link_libraries(bbInterface iTunesCOMWindowsSDK)

set_property(TARGET bbInterface PROPERTY FOLDER "plugins/bbInterface")

#INSTALL_IF_NEW = bbinterface.rc
#install(FILES changes.txt DESTINATION plugins/bbInterface)
install(TARGETS bbInterface RUNTIME DESTINATION plugins/bbInterface)
#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/bbInterface/${bbInterface_RESOURCES}")
#	install(FILES ${bbInterface_RESOURCES} DESTINATION plugins/bbInterface)
#endif()

