#include "BlackBox.h"
#include <blackbox/common.h>

namespace bb {

	struct BroamServer
	{
		HWND BBhwnd;
		bool HandleBroam (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void exec_command (wchar_t const * cmd);
		void execCommand (bbstring const & cmd);
		void post_command_fmt (const char * fmt, ...);
		void post_command (const char * cmd);
		void post_command (bbstring const & cmd);
		int exec_core_broam (wchar_t const * broam);
		bool exec_script (const wchar_t * broam);
		bool exec_broam (const wchar_t * broam);
	};
}

