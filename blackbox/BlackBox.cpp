#include "BlackBox.h"
#include "BlackBoxConfig.h"
#include "plugin/PluginConfig.h"
#include "plugin/PluginsConfig.h"
#include <yaml-cpp/yaml.h>
#include <bblib/codecvt.h>
#include "gfx/imgui.h"
#include "gfx/Gfx.h"
#include <bblib/logging.h>
#include <crazyrc/rc.h>
#include <net/Server.h>
#include <scheme/Scheme.h>
#include <net/commands.h>
#include <widgets/StyleEditor.h>
#include <widgets/Plugins.h>
#include <widgets/ControlPanel.h>
#include <widgets/Tasks.h>
#include <widgets/Pager.h>
#include <widgets/Debug.h>
#include "utils_win32.h"
#include "hooks/taskhook.h"

LRESULT CALLBACK mainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern "C"
{
	bb::BlackBox const * getBlackBoxInstance ()
	{
		return &bb::BlackBox::Instance();
	}

	bb::BlackBox * getBlackBoxInstanceRW ()
	{
		return &bb::BlackBox::Instance();
	}
}


#include <bblibcompat/styleprops.h>
void setDefaultValuesTo (StyleStruct & s)
{
	memset(&s, 0, sizeof(StyleStruct));

	COLORREF const c0 = 0x00996600;
	COLORREF const c1 = 0x00050505;

	COLORREF const tc0 = 0x00eee2b7;
	char const f0[] = "Verdana";
	wchar_t const f0W[] = L"Verdana";
	StyleItem tb = { 1, 1, 2, false, false, c0, c1, tc0, 15, 400, 1, 303, { "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false, { 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1), { L"Verdana" } };
	s.Toolbar = tb;

	StyleItem tbb = { 0, 0, 0, true, false, c0, c1, tc0, 0, 0, 0, 23, {"Verdana"}, 4, 0, 0, COLORREF(0), COLORREF(0), COLORREF(0), false, false, { 0 }, COLORREF(0), COLORREF(0), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.ToolbarButton = tbb;

	StyleItem tbbp = { 2, 1, 2, false, false, c0, c1, tc0, 0, 0, 0, 23, {"Verdana"}, 4, 0, 0, COLORREF(0), COLORREF(0), COLORREF(0), false, false,{ 0 }, COLORREF(0), COLORREF(0), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.ToolbarButtonPressed = tbbp;

	StyleItem tbl = { 2, 1, 2, false, false, c0, c1, tc0, 0, 0, 0, 15, { "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.ToolbarLabel = tbl;

	StyleItem tbwl = { 0, 0, 0, true, false, c0, c1, tc0, 0, 0, 0, 15,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.ToolbarWindowLabel = tbwl;

	StyleItem tbc = { 2, 1, 2, false, false, c0, c1, tc0, 0, 0, 0, 15,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.ToolbarClock = tbc;

	StyleItem mt = { 1, 1, 2, false, false, c0, c1, tc0, 15, 400, 1, 303,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.MenuTitle = mt;

	StyleItem mf = { 1, 1, 2, false, false, c0, c1, tc0, 15, 400, 0, 4399,{ "Verdana" }, 4, 0, 1, COLORREF(0), tc0, COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.MenuFrame = mf;

	StyleItem mh = { 1, 1, 2, false, false, c0, c1, tc0, 0, 0, 0, 15,{ "Verdana" }, 4, 0, 1, COLORREF(0), tc0, COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(0), COLORREF(-1),{ L"Verdana" } };
	s.MenuFrame = mh;

	StyleItem wtf = { 1, 1, 2, false, false, 0x00666666, c1, 0, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowTitleFocus = wtf;

	StyleItem wlf = { 2, 1, 2, false, false, 0x00996600, 0x00333300, tc0, 15, 400, 1, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowLabelFocus = wlf;

	StyleItem whf = { 1, 1, 2, false, false, 0x00666666, c1, 0, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(0), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowHandleFocus = whf;

	StyleItem wgf = { 0, 0, 0, true, false, 0, 0, 0, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(0), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowGripFocus = wgf;

	StyleItem wbf = { 0, 0, 0, true, false, c0, c1, tc0, 0, 0, 0, 17,{ "Verdana" }, 4, 0, 0, COLORREF(0), COLORREF(0), COLORREF(0), false, false,{ 0 }, COLORREF(0), COLORREF(0), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowButtonFocus = wbf;

	StyleItem wbp = { 2, 1, 2, false, false, 0x00cc9900, 0, 0x00ffffff, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 0, COLORREF(0), COLORREF(0), COLORREF(0), false, false,{ 0 }, COLORREF(0), COLORREF(0), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowButtonPressed = wbp;

	StyleItem wtu = { 1, 1, 2, false, false, 0x00666666, c1, 0, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(0), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowTitleUnfocus = wtu;

	StyleItem wlu = { 0, 0, 0, true, false, 0x00666666, c1, 0x00dddddd, 0, 0, 0, 15,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowLabelUnfocus = wlu;

	StyleItem whu = { 1, 1, 2, false, false, 0x00666666, c1, 0, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(0), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowHandleUnfocus = whu;

	StyleItem wgu = { 0, 0, 0, true, false, 0, 0, 0, 0, 0, 0, 7,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0), COLORREF(0), true, false,{ 0 }, COLORREF(0), COLORREF(-1), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowGripUnfocus = wgu;

	StyleItem wbu = { 0, 0, 0, true, false, 0x00666666, c1, tc0, 0, 0, 0, 17,{ "Verdana" }, 4, 0, 0, COLORREF(0), COLORREF(0), COLORREF(0), false, false,{ 0 }, COLORREF(0), COLORREF(0), COLORREF(-1), COLORREF(-1),{ L"Verdana" } };
	s.windowButtonUnfocus = wbu;

	s.windowFrameFocusColor = 0;
	s.windowFrameUnfocusColor = 0;
	s.menuAlpha = 255;
	s.toolbarAlpha = 255;
	s.menuNoTitle = false;
	s.borderColor = 0x000000;
	s.borderWidth = 1;
	s.bevelWidth = 1;
	s.frameWidth = 1;
	s.handleHeight = 5;
	//s.menuBullet = "";
	//s.menuBulletPosition ="";
	s.bulletUnix = true;
	s.metricsUnix = true;
	s.is_070 = false;
	s.menuTitleLabel = false;
	s.nix = false;
	StyleItem sl = { 1, 1, 2, false, false, c0, c1, tc0, 15, 400, 1, 47, { "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0xffffff), COLORREF(0x00b2a790), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(0), COLORREF(-1),{ L"Verdana" } };
	s.Slit = sl;
	s.MenuSepMargin = 0;
	s.MenuSepColor = 0;
	s.MenuSepShadowColor = 0;
	StyleItem mg = { 1, 1, 2, false, false, c0, c1, tc0, 15, 400, 1, 47,{ "Verdana" }, 4, 0, 1, COLORREF(0), COLORREF(0xffffff), COLORREF(0x00b2a790), true, false,{ 0 }, COLORREF(-1), COLORREF(-1), COLORREF(0), COLORREF(-1),{ L"Verdana" } };
	s.MenuGrip = mg;
}

namespace bb {

	BlackBox & BlackBox::Instance ()
	{
		static BlackBox instance;
		return instance;
	}

	BlackBox::BlackBox ()
		: m_hMainInstance(nullptr)
		, m_hwnd(nullptr)
		, m_taskHookWM(0)
		, m_taskHook32on64WM(0)
		, m_job(nullptr), m_inJob(false)
		, m_defaultStyle(new StyleStruct)
		, m_style()
		, m_explorer()
	{
		setDefaultValuesTo(*m_defaultStyle.get());
	}

	BlackBox::~BlackBox ()
	{
	}

	bool BlackBox::DetectConfig ()
	{
		bool ok = true;
		ok &= m_config.m_os.Init();
		ok &= m_config.m_display.Init();
		return ok;
	}

	bool BlackBox::LoadConfig ()
	{
		try
		{
			std::string cfg_name = m_cmdLine.YamlFile();
			TRACE_SCOPE_MSG(LL_INFO, CTX_BB | CTX_CONFIG | CTX_INIT, "Loading config file: %s", cfg_name.c_str());
			YAML::Node y_root = YAML::LoadFile(cfg_name);
			if (y_root.IsNull())
			{
				return false;
			}
//			YAML::Node y_bb = y_root["BlackBox"]; // @TODO: unicode? utf8?
//			if (y_bb)
//			{
//			}
			TasksConfig tasks_cfg;
			if (loadTasksConfig(y_root, tasks_cfg))
			{
				TRACE_MSG(LL_INFO, CTX_BB | CTX_CONFIG, "* loaded tasks section");
				m_config.m_tasks = tasks_cfg;
			}
			else
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "* failed to load tasks section");
			}

			WorkSpacesConfig ws_cfg;
			if (loadWorkSpacesConfig(y_root, ws_cfg))
			{
				TRACE_MSG(LL_INFO, CTX_BB | CTX_CONFIG, "* loaded workspaces section");
				m_config.m_wspaces = ws_cfg;
			}
			else
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "* failed to load tasks section");
			}

			TrayConfig tray_cfg;
			if (loadTrayConfig(y_root, tray_cfg))
			{
				TRACE_MSG(LL_INFO, CTX_BB | CTX_CONFIG, "* loaded tray section");
				m_config.m_tray = tray_cfg;
			}
			else
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "* failed to load tray section");
			}

			PluginsConfig plugins_cfg;
			if (loadPluginsConfig(y_root, plugins_cfg))
			{
				TRACE_MSG(LL_INFO, CTX_BB | CTX_CONFIG, "* loaded plugins section");
				m_config.m_plugins = plugins_cfg;
			}
			else
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_CONFIG, "* failed to load plugins section");
			}

			TRACE_MSG(LL_INFO, CTX_BB | CTX_CONFIG, "Config loaded");
			return true;
		}
		catch (std::exception & e)
		{
			TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "exception caught: %s", e.what());
			return false;
		}
	}

	bool BlackBox::CreateBBWindow ()
	{
		m_hwnd = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			s_blackboxClass,
			s_blackboxName,
			WS_POPUP | WS_DISABLED,
			// sizes are assigned for cursor behaviour with
			// AutoRaise Focus on winME, win2k
			0,//getWorkspaces().GetVScreenX(),
			0, //getWorkspaces().GetVScreenY(),
			1024, //getWorkspaces().GetVScreenWidth(),
			768, //getWorkspaces().GetVScreenHeight(),
			NULL,
			NULL,
			m_hMainInstance,
			NULL
			);

		DWORD const res = ::GetLastError();
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_INIT, "BlackBox hwnd=0x%x code=0x%x", m_hwnd, res);
		return m_hwnd != nullptr;
	}


	struct SecondMon
	{
		int x0, y0, x1, y1;
		bool found;
	};

	BOOL CALLBACK MonitorEnumProc (HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		MONITORINFOEX iMonitor;
		iMonitor.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &iMonitor);

		SecondMon * s = reinterpret_cast<SecondMon *>(dwData);
		if (s->x0 == 0 && s->y0 == 0)
		{
			if (iMonitor.rcMonitor.left == 0 && iMonitor.rcMonitor.top == 0)
				return true; // first monitor

			s->x0 = iMonitor.rcMonitor.left;
			s->y0 = iMonitor.rcMonitor.top;
			s->x1 = iMonitor.rcMonitor.right;
			s->y1 = iMonitor.rcMonitor.bottom;
			s->found = true;
			return false; // abort enum
		}
		return true;
	}

	bool BlackBox::Init (HINSTANCE hmi)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
		m_hMainInstance = hmi;
		bool ok = true;

		ok &= DetectConfig();
		ok &= mkJobObject(m_job, m_inJob);
		ok &= m_cmdLine.Init();
		ok &= LoadConfig();
		ok &= m_scheme.Init(m_config.m_scheme);

		m_taskHookWM = ::RegisterWindowMessage(c_taskHookName);
		m_taskHook32on64WM = ::RegisterWindowMessage(c_taskHook32Name);
		std::unique_ptr<Explorer> e(new Explorer);
		m_explorer = std::move(e);

		rc::init();

		Win32RegisterClass(s_blackboxClass, mainWndProc, 0);
		ok &= CreateBBWindow();
		ok &= m_wspaces.Init(m_config.m_wspaces);
		ok &= m_gfx.Init();
		ok &= m_tasks.Init(m_config.m_tasks);
		ok &= m_explorer->Init();

		SecondMon s = { 0 };
		EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, reinterpret_cast<LPARAM>(&s));
		int const szx = 128;
		int sx = szx;
		if (s.found)
		{
			sx = s.x1;
		}
//		GfxWindow * w0 = m_gfx.MkGuiWindow(sx - szx, s.y0, szx, szx, L"c0", L"w0");
//		{
//			w0->m_gui->m_enabled = true;
//			DebugWidget * w0wdg0 = new DebugWidget;
//			w0wdg0->m_enabled = true;
//			w0->GetGui()->AddWidget(w0wdg0);
//		}

		{
			GfxWindow * w1 = m_gfx.MkGuiWindow(0, 200, 800, 600, L"bbTasks", L"bbTasks");
			w1->m_gui->m_enabled = true;
			TasksWidget * w1wdg0 = new TasksWidget;
			w1wdg0->m_enabled = true;
			w1wdg0->m_horizontal = false;
			w1->GetGui()->AddWidget(w1wdg0);
		}

		GfxWindow * w0 = m_gfx.MkGuiWindow(0, 0, 400, 200, L"bbPager", L"bbPager");
		{
			w0->m_gui->m_enabled = true;
			PagerWidget * w0wdg0 = new PagerWidget;
			w0wdg0->m_enabled = true;
			w0->GetGui()->AddWidget(w0wdg0);
		}

//		GfxWindow * w1 = m_gfx.MkGuiWindow(0, 200, 800, 600, L"bbStyleEditor", L"bbStyleEditor");
//		{
//			w1->m_gui->m_enabled = true;
//			StyleEditorWidget * w1wdg0 = new StyleEditorWidget;
//			w1wdg0->m_enabled = true;
//			w1->GetGui()->AddWidget(w1wdg0);
//		}

		ok &= m_plugins.Init(m_config.m_plugins);
		ok &= m_server.Init(m_config.m_server);
		return ok;
	}

	bool BlackBox::Done ()
	{
		TRACE_SCOPE_MSG(LL_INFO, CTX_BB, "Terminating BB");
		bool ok = true;
		ok &= m_server.Done();
		ok &= m_scheme.Done();
		ok &= m_plugins.Done();
		ok &= m_tasks.Done();
		ok &= m_gfx.Done();
		ok &= m_tray.Done();
		ok &= m_explorer->Done();

		::DestroyWindow(m_hwnd);
		m_hwnd = nullptr;
		::UnregisterClass(s_blackboxClass, m_hMainInstance);
		::CloseHandle(m_job);
		m_job = nullptr;
		return ok;
	}

	bool BlackBox::Win32RegisterClass (const wchar_t * classname, WNDPROC wndproc, int flags)
	{
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.hInstance = m_hMainInstance;
		wc.lpszClassName = classname;
		wc.lpfnWndProc = wndproc;

		//		if (flags & BBCS_VISIBLE)
		//		{
		//			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		//			wc.style |= CS_DBLCLKS;
		//		}
		//		if ((flags & BBCS_DROPSHADOW) && g_usingXP)
		//			wc.style |= CS_DROPSHADOW;
		//		if (flags & BBCS_EXTRA)
		//			wc.cbWndExtra = sizeof(void*);
		if (::RegisterClass(&wc))
			return true;
		//BBMessageBox(MB_OK, NLS2("$Error_RegisterClass$", "Error: Could not register \"%s\" window class."), classname);
		return false;
	}

	void on_crash_handler ()
	{
		// getWorkspaces().GatherWindows()
	}

	void BlackBox::Run ()
	{
		MSG msg;
		msg.wParam = 0;
		//TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Entering main message loop...");
		__try
		{
			/* Main message loop */
			for (;;)
			{
				DWORD timeout = 16;
				if (::MsgWaitForMultipleObjects(0, NULL, FALSE, timeout, QS_ALLINPUT) == WAIT_OBJECT_0)
				{
					while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}

				m_gfx.NewFrame();

				m_tasks.Update();
				HandleServerMessages();

				if (!m_cmdLine.NoTrayHook())
					m_tray.UpdateFromTrayHook();

				// Rendering
				m_gfx.Render();
			}

			m_gfx.Done();
		}
		/* On crash: gather windows, then pass it to the OS */
		__except (on_crash_handler(), EXCEPTION_CONTINUE_SEARCH)
		{
		}
		//TRACE_MSG(trace::e_Info, trace::CTX_BBCore, "Main message loop terminated...");
	}

	void BlackBox::HandleServerMessages ()
	{
		m_server.m_requestLock.Lock();
		
		for (size_t i = 0, ie = m_server.m_requests.size(); i < ie; ++i)
		{
			std::unique_ptr<PendingCommand> req = std::move(m_server.m_requests[i]);
			if (req)
			{
				std::unique_ptr<Command> resp = HandleServerMessage(req->m_request);
				if (resp)
				{
					req->m_response = std::move(resp);
					m_server.m_responses.push_back(std::move(req));
				}
			}
		}

		m_server.m_requestLock.Unlock();

		m_server.DispatchResponses();
	}

	std::unique_ptr<Command> BlackBox::HandleServerMessage (std::unique_ptr<Command> const & request)
	{
		switch (request->GetType())
		{
			case E_CommandType::e_bbcmd:
			{
				Command_bbcmd const * const r = static_cast<Command_bbcmd const *>(request.get());
				char response[4096];
				m_scheme.Eval(r->m_rawcmd.c_str(), response, 4096);
				return std::unique_ptr<Command>();
//				return std::unique_ptr<Command>(new Command_bbcmd_ack(m_hwnd));
			}
			default:
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_NET, "Unknown command");
				return nullptr;
			}
		}
	}
}
