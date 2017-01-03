#include <3rd_party/tclap/CmdLine.h>
#include <memory>

namespace bb {

	struct CommandLine
	{
		TCLAP::ValueArg<std::string> m_cmd;
		TCLAP::ValueArg<std::string> m_host;
		TCLAP::ValueArg<int> m_port;
		std::unique_ptr<TCLAP::CmdLine> m_cmdLine;
		bool m_quit;

		CommandLine ()
			: m_cmd("c", "command", "Run command on server", false, "", "scheme command")
			, m_host("a", "addr", "Specify addr", false, "127.0.0.1", "ip addr")
			, m_port("p", "port", "Specify port", false, 13199, "tcp port")
			, m_quit(false)
		{ }

		bool Init ();
		bool Done ();

		std::string const & Cmd () const { return m_cmd.getValue(); }
		std::string const & Host () const { return m_host.getValue(); }
		int const & Port () const { return m_port.getValue(); }
	};

}
