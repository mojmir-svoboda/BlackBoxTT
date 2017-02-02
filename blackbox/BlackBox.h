#pragma once
#include <platform_win.h>
#include "Tasks.h"
#include "WorkSpaces.h"
#include "Tray.h"
#include "Explorer.h"
#include "gfx/Gfx.h"
#include "CommandLine.h"
#include "DesktopWallpaper.h"
#include <net/Server.h>
#include <scheme/Scheme.h>
#include <VersionHelpers.h>
#include "BlackBoxConfig.h"
#include <plugin/PluginManager.h>
#include "blackbox_api.h"
#include <bblibcompat/StyleStruct.h>
namespace bb { struct MenuWidget; }
namespace YAML { class Node; }

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

		BlackBox ();
		~BlackBox ();
		BB_API static BlackBox & Instance ();
		BlackBox (BlackBox const &) = delete;
		BlackBox & operator= (BlackBox const &) = delete;

		BB_API bool Init (HINSTANCE hInstance);
		BB_API bool Done ();
		BB_API void Run ();
		bool CreateBBWindow ();
		bool HomeDir (wchar_t * cfgpath, size_t sz) const;
		bool LoadConfig ();
		bool FindConfig (wchar_t * cfgpath, size_t sz, const wchar_t * cfgfile) const;

		HINSTANCE GetHInstance () const { return m_hMainInstance; }
		HANDLE GetJob () const { return m_job; }
		bool GetInJob () const { return m_inJob; }
		Tasks & GetTasks () { return m_tasks; }
		Tasks const & GetTasks () const { return m_tasks; }
		WorkSpaces & GetWorkSpaces() { return m_wspaces; }
		WorkSpaces const & GetWorkSpaces() const { return m_wspaces; }
		Tray & GetTray () { return m_tray; }
		Tray const & GetTray () const { return m_tray; }
		bool CreateGfx (GfxConfig & cfg);
		Gfx & GetGfx () { return *m_gfx; }
		Gfx const & GetGfx () const { return *m_gfx; }
		Explorer & GetExplorer () { return *m_explorer; }
		Explorer const & GetExplorer () const { return *m_explorer; }

		MenuWidget * CreateMenu (WidgetConfig & wcfg, MenuConfig const & config);
		MenuWidget * CreateMenuOnPointerPos (MenuConfig const & config);
		void ShowMenuOnPointerPos (bool show);
		bool AddIconToCache (bbstring const & name, HICON ico, IconId & id);
		bool FindIconInCache (bbstring const & name, IconId & id);

		// probably future binds
		bool GetPointerPos (int & x, int & y);
		// binds
		void Quit (uint32_t arg);
		void ShowMenu (bbstring const & vertex_id);
		void ToggleMenu (bbstring const & vertex_id);
		void MakeSticky (HWND hwnd);
		void RemoveSticky (HWND hwnd);
		HWND GetHwnd ();
		unsigned GetTaskHook32on64WM () const { return m_taskHook32on64WM; }
		unsigned GetTaskHookWM () const { return m_taskHookWM; }
		void * GetSettingPtr (int sn_index);
		bool GetConfigDir (wchar_t * dir, size_t dir_sz) const;

		bool FindTargetVertexViaEdge (bbstring const & edge_property, bbstring & dst_vertex_id) const;
		bool MoveWindowToVertex (HWND hwnd, bbstring const & dst_vertex_id);
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
		/**@fn SetTaskManIgnored
		 * @param [in] operation: true, false, toggle
		 **/
		void SetTaskManIgnored (bbstring const & op);

	protected:
		friend LRESULT CALLBACK mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool DetectConfig ();
		bool Win32RegisterClass (wchar_t const * classname, WNDPROC wndproc, int flags);
		void HandleServerMessages ();
		std::unique_ptr<Command> HandleServerMessage (std::unique_ptr<Command> const & request);
		HWND FindTopLevelWindow () const;

	protected:
		HINSTANCE m_hMainInstance { nullptr };
		HWND m_hwnd { nullptr };
		unsigned m_taskHookWM { 0 };
		unsigned m_taskHook32on64WM { 0 };
		HANDLE m_job { nullptr };
		bool m_inJob { false };
		bool m_quit { false };
		std::unique_ptr<YAML::Node> m_y_config;
		BlackBoxConfig m_config;
		CommandLine m_cmdLine;
		Scheme m_scheme;
		Server m_server;

		std::unique_ptr<StyleStruct> m_defaultStyle; /// legacy style
		std::unique_ptr<StyleStruct> m_style; /// legacy style
		std::unique_ptr<Explorer> m_explorer;
		WorkSpaces m_wspaces;
		Tasks m_tasks;
		Tray m_tray;
		std::unique_ptr<Gfx> m_gfx;
		PluginManager m_plugins;
		DesktopWallpaper m_wallpapers;
		MenuWidget * m_menuWidget { nullptr };
	};
}

extern "C"
{
	BB_API bb::BlackBox const * getBlackBoxInstance ();
	BB_API bb::BlackBox * getBlackBoxInstanceRW ();
}

