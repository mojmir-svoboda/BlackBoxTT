#include <platform_win.h>
#include <hooks/taskhook.h>
#include <3rd_party/tclap/CmdLine.h>

struct BB32CommandLine
{
	TCLAP::SwitchArg m_notaskhook;
	TCLAP::SwitchArg m_notrayhook;
	TCLAP::ValueArg<std::string> m_bbHwnd;
	TCLAP::ValueArg<std::string> m_wmTaskHook;
	TCLAP::CmdLine * m_cmdLine = { nullptr };

	BB32CommandLine ();
	bool Init ();
	bool Done ();

	bool NoTaskHook () { return m_notaskhook.getValue(); }
	bool NoTrayHook () { return m_notrayhook.getValue(); }
	std::string const & GetbbHwnd () const { return m_bbHwnd.getValue(); }
	std::string const & GetwmTaskHook () const { return m_wmTaskHook.getValue(); }
};

BB32CommandLine::BB32CommandLine()
	: m_notaskhook("t", "notaskhook", "Do not run task hook dll", false)
	, m_notrayhook("y", "notrayhook", "Do not run tray hook dll", false)
	, m_bbHwnd("b", "bbhwnd", "BB hwnd.", false, "", "handle")
	, m_wmTaskHook("w", "wmtaskhook", "wm message", false, "", "handle")
	, m_cmdLine(nullptr)
{
}

bool BB32CommandLine::Init ()
{
	try
	{
		//TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "BB32 Initializing command line");
		std::string version; // @TODO: replace by bbversion
		if (m_cmdLine = new TCLAP::CmdLine("BlackBox32on64 for Windows64", ' ', version))
		{
			m_cmdLine->setExceptionHandling(false);
			m_cmdLine->add(m_notrayhook);
			m_cmdLine->add(m_notaskhook);
			m_cmdLine->add(m_bbHwnd);
			m_cmdLine->add(m_wmTaskHook);
		
			//ValueArg<int> itest("i", "intTest", "integer test", true, 5, "int");

			m_cmdLine->parse(__argc, __argv);
			return true;
		}
	}
	catch (TCLAP::ArgException & e)
	{
		//TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse error: %s at arg=%i", e.error().c_str(), e.argId());
		return false;
	}
	catch (TCLAP::ExitException & e)
	{
		//TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse exception: status=%i", e.getExitStatus());
		return false;
	}
	catch (...)
	{
		//TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse exception: status");
		return false;
	}
	return true;
}

bool BB32CommandLine::Done ()
{
	//TRACE_MSG(LL_INFO, CTX_BB, "Terminating command line");
	delete m_cmdLine;
	m_cmdLine = nullptr;
	return true;
}



// https://blogs.msdn.microsoft.com/oldnewthing/20031211-00/?p=41543
#include <Shlwapi.h>
bool initTaskHook32 (BB32CommandLine const & cli)
{
	std::string const hwnd_str = cli.GetbbHwnd();
	LONGLONG llHandle;
	HWND bbHwnd = nullptr;
	if (StrToInt64ExA(hwnd_str.c_str(), STIF_DEFAULT, &llHandle))
	{
		bbHwnd = (HWND)(INT_PTR)llHandle;
	}

	std::string const wm_str = cli.GetwmTaskHook();
	unsigned wm = 0;
	int iwm = 0;
	if (StrToIntExA(wm_str.c_str(), STIF_DEFAULT, &iwm))
	{
		wm = static_cast<unsigned>(iwm);
	}

	if (bbHwnd && wm)
	{
		return initTaskHook32(bbHwnd, wm);
	}
	return false;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pszCmdLine, int iCmdShow)
{
	system("pause");
	BB32CommandLine cli;
	cli.Init();

	if (!cli.NoTaskHook())
		initTaskHook32(cli);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) && WM_QUIT != msg.message)
	{
  	TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  doneTaskHook();
	cli.Done();
  return static_cast<int>(msg.wParam);
}

