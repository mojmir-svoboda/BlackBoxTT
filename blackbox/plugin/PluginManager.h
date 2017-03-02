/* ==========================================================================

  This file is part of the bbLean source code
  Copyright © 2001-2003 The Blackbox for Windows Development Team
  Copyright © 2004-2009 grischka
  Copyright © .. mojmir
  ========================================================================== */
#pragma once
#include "PluginsConfig.h"
#include "plugin.h"
#include <memory>

#define SUB_PLUGIN_LOAD 1
#define SUB_PLUGIN_SLIT 2
namespace bb {

	enum plugin_errors : int
	{
		error_plugin_success           = 0,
		error_plugin_is_built_in       = 1,
		error_plugin_dll_not_found     ,
		error_plugin_dll_needs_module  ,
		error_plugin_does_not_load     ,
		error_plugin_missing_entry     ,
		error_plugin_fail_to_load      ,
		error_plugin_crash_on_load     ,
		error_plugin_crash_on_unload   ,
		error_plugin_message
	};

	struct PluginInfo
	{
		HMODULE m_module;

		PluginConfig m_config;

		bool m_enabled;			// plugin should be loaded
		bool m_isWideChar;  // plugin has pluginInfoW function
		bool m_canUseSlit;  // has slit method
		bool m_inSlit;			// is configured to be in slit
		bool m_isSlit;

		int m_instance; // if the same plugin name is used more than once

		beginPlugin_t m_beginPlugin;
		beginPluginEx_t m_beginPluginEx;
		beginSlitPlugin_t m_beginSlitPlugin;
		endPlugin_t m_endPlugin;
		pluginInfo_t m_pluginInfo;
		pluginInfoW_t m_pluginInfoW;

		PluginInfo () : m_module(nullptr), m_enabled(false), m_isWideChar(false), m_canUseSlit(false), m_inSlit(false), m_isSlit(false), m_instance(0), m_beginPlugin(nullptr), m_beginPluginEx(nullptr), m_beginSlitPlugin(nullptr), m_endPlugin(nullptr), m_pluginInfo(nullptr) { }

		plugin_errors LoadPlugin (HWND hSlit);
		bool UnloadPlugin ();
		void InitFromConfig (PluginConfig const & pc)
		{
			m_config = pc;
			m_inSlit = pc.m_inSlit;
		}
	};

	using PluginInfoPtr = std::unique_ptr<PluginInfo>;

	struct PluginInfos
	{
		std::vector<PluginInfoPtr> m_infos;
	};	


  struct PluginManager
  {
		PluginsConfig m_config;
		PluginInfos m_infos;
		HWND m_hSlit { nullptr };

    bool Init (PluginsConfig const & cfg);
		bool Unload ();
    bool Done ();
		void LoadPlugin (bbstring const & plugin_id);
		void UnloadPlugin (bbstring const & plugin_id);
		bool IsPluginLoaded (bbstring const & plugin_id) const;

    void AboutPlugins ();
    // handle the "@BBCfg.plugin.xxx" bro@ms from the config->plugins menu
    int HandleBroam (const char * submessage);
    //Menu * GetMenu (const char * text, char * menu_id, bool pop, int mode);
	protected:

  };
}

