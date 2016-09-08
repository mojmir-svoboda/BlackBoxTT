#pragma once
#include <platform_win.h>
#include "Tasks.h"
#include "Widgets.h"
#include "WorkSpaces.h"
#include "Tray.h"
#include "Explorer.h"
#include "gfx/Gfx.h"
#include "CommandLine.h"
#include <net/Server.h>
#include <scheme/Scheme.h>
#include <VersionHelpers.h>
#include "BlackBoxConfig.h"
#include <plugin/PluginManager.h>
#include "blackbox_api.h"
#include <bblibcompat/StyleStruct.h>

namespace bb {

	struct /*BB_API*/ BlackBox
	{
		constexpr static wchar_t const * const s_blackboxName = L"BlackBox";
		constexpr static char const * const s_blackboxConfig = "blackbox.yaml";
		constexpr static wchar_t const * const s_blackboxHomeDir = L".blackboxTT";
		constexpr static wchar_t const * const s_blackboxEtcDir = L"etc";
		//constexpr static char const * const s_BlackboxClass = "BlackBoxClass";
		constexpr static wchar_t const * const s_blackboxClass = L"BlackBoxClass";
		constexpr static wchar_t const * const s_blackbox32Name = L"blackbox32.exe";
		HINSTANCE m_hMainInstance;
		HWND m_hwnd;
		unsigned m_taskHookWM;
		unsigned m_taskHook32on64WM;
		HANDLE m_job;
		bool m_inJob;
		BlackBoxConfig m_config;
		CommandLine m_cmdLine;
    Scheme m_scheme;
		Server m_server;

		std::unique_ptr<StyleStruct> m_defaultStyle; /// legacy style
		std::unique_ptr<StyleStruct> m_style; /// legacy style
		std::unique_ptr<Explorer> m_explorer;
		WorkSpaces m_wspaces;
		Tasks m_tasks;
		Widgets m_widgets;
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
		bool FindConfig (wchar_t * cfgpath, size_t sz, const wchar_t * cfgfile) const;
		bool DetectConfig ();
		BB_API bool Done ();
		bool Win32RegisterClass (wchar_t const * classname, WNDPROC wndproc, int flags);
		BB_API void Run ();
		void HandleServerMessages ();
		std::unique_ptr<Command> HandleServerMessage (std::unique_ptr<Command> const & request);

		HANDLE GetJob () const { return m_job; }
		bool GetInJob () const { return m_inJob; }
		Tasks & GetTasks () { return m_tasks; }
		Tasks const & GetTasks () const { return m_tasks; }
		WorkSpaces & GetWorkSpaces() { return m_wspaces; }
		WorkSpaces const & GetWorkSpaces() const { return m_wspaces; }
		Tray & GetTray () { return m_tray; }
		Tray const & GetTray () const { return m_tray; }
		Gfx & GetGfx () { return m_gfx; }
		Gfx const & GetGfx () const { return m_gfx; }

		HWND FindTopLevelWindow () const;

		// binds
		void Quit (uint32_t arg);
		void MakeSticky (HWND hwnd);
		void RemoveSticky (HWND hwnd);
		HWND GetHwnd ();
		unsigned GetTaskHook32on64WM () const { return m_taskHook32on64WM; }
		unsigned GetTaskHookWM () const { return m_taskHookWM; }
		void * GetSettingPtr (int sn_index);
		bool GetConfigDir (wchar_t * dir, size_t dir_sz) const;

		bool FindTargetVertexViaEdge (bbstring const & edge_property, bbstring & dst_vertex_id) const;
		bool MoveTopWindowToVertexViaEdge (bbstring const & edge_property);
		/**@fn WorkSpacesSetCurrentVertexId
		 * @brief finds vertex_id in the graph and switches to it. no edge from current is required.
		 **/
		bool WorkSpacesSetCurrentVertexId (bbstring const & vertex_id);
		/**@fn WorkSpacesSwitchVertexViaEdge
		 * @brief uses edge with 'edge_property' to go from current vertex to destination vertex
		 **/
		bool WorkSpacesSwitchVertexViaEdge (bbstring const & edge_property);
		/**@fn MaximizeTopWindow
		 * @param [in] vertical
		 **/
		void MaximizeTopWindow (bool vertical);
	};
}

extern "C"
{
	BB_API bb::BlackBox const * getBlackBoxInstance ();
	BB_API bb::BlackBox * getBlackBoxInstanceRW ();
}

