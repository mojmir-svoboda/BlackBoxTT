#pragma once
#include <vector>
#include <memory>
#include <bblib/bbstring.h>
#include "Gui.h"

namespace bb {

	LRESULT CALLBACK Gui::GuiWndProcDispatch (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		Gui * gui = nullptr;

		if (msg == WM_NCCREATE)
		{
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
			gui = reinterpret_cast<Gui *>(lpcs->lpCreateParams);
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(gui));
			//assert(gui->m_hwnd == gui->SetHWND(hwnd);
		}
		else
		{
			gui = reinterpret_cast<Gui *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (gui)
			return gui->WndProcHandler(hwnd, msg, wparam, lparam);
		else
			return DefWindowProc(hwnd, msg, wparam, lparam);;
	}
}

