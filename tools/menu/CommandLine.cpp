#include "CommandLine.h"
//#include <bblib/logging.h>

namespace bb {

	bool CommandLine::Init ()
	{
		try
		{
			//TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "bbcli: Initializing command line");
			std::string version; // @TODO: replace by bbversion
			m_cmdLine.reset(new TCLAP::CmdLine("bbcli for BlackBox for Windows", ' ', version));
			if (m_cmdLine)
			{
				m_cmdLine->setExceptionHandling(false);
				m_cmdLine->add(m_cmd);
				m_cmdLine->add(m_host);
				m_cmdLine->add(m_port);

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
			return false;
		}
		catch (...)
		{
			//TRACE_MSG(LL_ERROR, CTX_INIT | CTX_CONFIG, "Command line parse exception: status");
			return false;
		}
		return true;
	}

	bool CommandLine::Done ()
	{
		return true;
	}
}

