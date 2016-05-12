#include <3rd_party/tclap/CmdLine.h>

namespace bb {

	struct CommandLine
	{
		TCLAP::SwitchArg m_nostartup;
		TCLAP::SwitchArg m_notaskhook;
		TCLAP::SwitchArg m_notrayhook;
		TCLAP::ValueArg<std::string> m_configdir;
		TCLAP::ValueArg<std::string> m_rcfile;
		TCLAP::ValueArg<std::string> m_yamlfile;
		TCLAP::ValueArg<std::string> m_exec;
		TCLAP::CmdLine * m_cmdLine;

		CommandLine ();
		bool Init ();
		bool Done ();

		bool NoStartup () { return m_nostartup.getValue(); }
		bool NoTaskHook () { return m_notaskhook.getValue(); }
		bool NoTrayHook () { return m_notrayhook.getValue(); }
		std::string const & YamlFile () const { return m_yamlfile.getValue(); }
		std::string const & ConfigDir () const { return m_configdir.getValue(); }
	};

}
