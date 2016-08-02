#include "commands.h"
#include <bbproto/decoder.h>
#include <bblib/logging.h>

namespace bb {

	std::unique_ptr<Command> mkCommand (DecodedCommand const & cmd)
	{
		switch (cmd.present)
		{
			//case Command_PR_bb32wm: return std::unique_ptr<Command>(new Command_bb32wm(static_cast<unsigned>(cmd.choice.bb32wm.wmmsg)));
			case Command_PR_bbcmd: return std::unique_ptr<Command>(new Command_bbcmd(cmd.choice.bbcmd.cmd));
			default:
			{
				TRACE_MSG(LL_ERROR, CTX_BB | CTX_NET, "Unknown command");
				return nullptr;
			}
		}
	}

// 	size_t Command_bb32wm_ack::Encode (char * buff, size_t buffsz)
// 	{
// 
// 		return 0;
// 	}
}


