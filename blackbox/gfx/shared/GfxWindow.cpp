#include "GfxWindow.h"
#include "Gfx.h"
#include "DX11.h"
#include <blackbox/gfx/Gui.h>
#include <blackbox/utils_window.h>

namespace bb {
namespace shared {

	void GfxWindow::NewFrame ()
	{
		m_gui->NewFrame();
	}

	void GfxWindow::Render ()
	{
		if (m_chain && m_view)
		{
			m_gui->Render();	
			m_chain->Present(1, 0);
		}
	}

	void GfxWindow::Show (bool on)
	{
		m_gui->Show(on);
		showWindow(m_hwnd, on);
	}

	bool GfxWindow::Visible () const
	{
		return m_gui->Visible();
	}

	bool GfxWindow::Done ()
	{
		if (m_gui)
		{
			m_gui->Done();
			m_view = nullptr;
		}

		if (m_view)
		{
			m_view->Release();
			m_view = nullptr;
		}
		if (m_chain)
		{
			m_chain->Release();
			m_chain = nullptr;
		}
		if (m_hwnd)
		{
			::DestroyWindow(m_hwnd);
			m_hwnd = nullptr;
			//::UnregisterClass(m_clName.c_str(), nullptr);
		}
		return true;
	}

}}
