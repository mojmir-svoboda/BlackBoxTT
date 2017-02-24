#include "BlackBox.h"
#include <blackbox/common.h>
enum : size_t { e_broamMsgLenMax = 1024 };

namespace bb {

	struct BroamServer
	{
		HWND m_BBHwnd { nullptr };
		std::vector<HWND> m_listeners;

		bool Init (HWND bb_hwnd) { m_BBHwnd = bb_hwnd; return true; }
		bool IsBroam (uint32_t uMsg) const;

		bool HandleWndMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool HandleCoreBroam (HWND hwnd, uint32_t uMsg, WPARAM wParam, wchar_t const * msg);
		bool BroadcastBBMessage (UINT msgType, WPARAM wParam, LPARAM lParam);

		bool PostCommand (wchar_t const * fmt, ...);
		void PostCommand (bbstring const & cmd);
	protected:
		bool RegisterBroamListener (HWND handle, uint32_t const * msgs);
		bool UnregisterBroamListener (HWND handle, uint32_t const * msgs);
		void exec_command (wchar_t const * cmd);
		void execCommand (bbstring const & cmd);
		int exec_core_broam (wchar_t const * broam);
		bool exec_script (const wchar_t * broam);
		bool exec_broam (const wchar_t * broam);
	};
}

