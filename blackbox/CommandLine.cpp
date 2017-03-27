#include "CommandLine.h"
#include <blackbox/common.h>

namespace bb {

	CommandLine::CommandLine ()
		: m_nostartup("n", "nostartup", "Do not run startup programs", false)
		, m_taskhook("t", "taskhook", "Run task hook dll (dll injection)", false)
		, m_trayhook("y", "trayhook", "Run tray hook dll (dll injection)", false)
		, m_configdir("d", "dir", "Specify directory with config files.", false, "", "directory")
		, m_logdir("l", "logdir", "Specify directory for log files.", false, "", "directory")
		, m_unbuffered("u", "unbuffered", "Logging output is unbuffered.", false)
		, m_rcfile("r", "rcfile", "Specify config file.", false, "blackbox.rc", "rc config file")
		, m_yamlfile("f", "yaml", "Specify config file.", false, "blackbox.yaml", "yaml config file")
		, m_exec("e", "exec", "Send broadcast message to running WM", false, "@broam", "@broam")
		, m_cmdLine(nullptr)
	{
	}

	bool CommandLine::Init ()
	{
		try
		{
			std::string version; // @TODO: replace by bbversion
			if (m_cmdLine = new TCLAP::CmdLine("BlackBox for Windows", ' ', version))
			{
				m_cmdLine->setExceptionHandling(false);
				m_cmdLine->add(m_nostartup);
				m_cmdLine->add(m_trayhook);
				m_cmdLine->add(m_taskhook);
				m_cmdLine->add(m_configdir);
				m_cmdLine->add(m_logdir);
				m_cmdLine->add(m_rcfile);
				m_cmdLine->add(m_yamlfile);
				m_cmdLine->add(m_exec);
				m_cmdLine->add(m_unbuffered);

				//ValueArg<int> itest("i", "intTest", "integer test", true, 5, "int");

				m_cmdLine->parse(__argc, __argv);
				return true;
			}
		}
		catch (TCLAP::ArgException & e)
		{
			TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse error: %s at arg=%i", e.error().c_str(), e.argId());
			return false;
		}
		catch (TCLAP::ExitException & e)
		{
			TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse exception: status=%i", e.getExitStatus());
			return false;
		}
		catch (...)
		{
			TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse exception: status");
			return false;
		}
		return true;
	}

	bool CommandLine::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB, "Terminating command line");
		delete m_cmdLine;
		m_cmdLine = nullptr;
		return true;
	}
}

