#pragma once
#include <platform_win.h>
#include "Tasks.h"
#include "Tray.h"
#include "Explorer.h"
#include "gfx/Gfx.h"
#include "CommandLine.h"

#include <VersionHelpers.h>
#include "BlackBoxConfig.h"
#include <plugin/PluginManager.h>
#include "blackbox_api.h"
#include <bblibcompat/StyleStruct.h>

namespace bb {

	struct /*BB_API*/ BlackBox
	{
		constexpr static wchar_t const * const s_BlackboxName = L"BlackBox";
		constexpr static char const * const s_BlackboxConfig = "blackbox.yaml";
		//constexpr static char const * const s_BlackboxClass = "BlackBoxClass";
		constexpr static wchar_t const * const s_BlackboxClass = L"BlackBoxClass";
		HINSTANCE m_hMainInstance;
		HWND m_hwnd;
		BlackBoxConfig m_config;
		CommandLine m_cmdLine;

		std::unique_ptr<StyleStruct> m_defaultStyle; /// legacy style
		std::unique_ptr<StyleStruct> m_style; /// legacy style
		std::unique_ptr<Explorer> m_explorer;
		Tasks m_tasks;
		Tray m_tray;
		Gfx m_gfx;
		PluginManager m_plugins;

		BlackBox ();
		~BlackBox ();
		BB_API static BlackBox & Instance ();
		BlackBox (BlackBox const &) = delete;
		BlackBox & operator= (BlackBox const &) = delete;

		BB_API bool Init (HINSTANCE hInstance);
		bool CreateBBWindow ();
		bool LoadConfig ();
		bool DetectConfig ();
		BB_API bool Done ();
		bool Win32RegisterClass (wchar_t const * classname, WNDPROC wndproc, int flags);
		BB_API void Run ();

		Tasks & GetTasks () { return m_tasks; }
		Tasks const & GetTasks () const { return m_tasks; }
		Tray & GetTray () { return m_tray; }
		Tray const & GetTray () const { return m_tray; }
		Gfx & GetGfx () { return m_gfx; }
		Gfx const & GetGfx () const { return m_gfx; }

		// binds
		void Quit (uint32_t arg);
		void MakeSticky (HWND hwnd);
		void RemoveSticky (HWND hwnd);
		HWND GetHwnd ();
		void * GetSettingPtr (int sn_index);
		bool GetConfigDir (wchar_t * dir, size_t dir_sz) const;
	};
}

extern "C"
{
	BB_API bb::BlackBox const * getBlackBoxInstance ();
	BB_API bb::BlackBox * getBlackBoxInstanceRW ();
}

