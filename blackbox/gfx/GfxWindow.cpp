#include "GfxWindow.h"
#include "DX11.h"
#include "Gui.h"

namespace bb {

	void GfxWindow::NewFrame ()
	{
		m_gui->NewFrame();
	}

	void GfxWindow::Render ()
	{
		m_gui->m_dx11->m_pd3dDeviceContext->OMSetRenderTargets(1, &m_view, nullptr);
		m_gui->m_dx11->m_pd3dDeviceContext->ClearRenderTargetView(m_view, (float*)&m_clrCol);
		m_gui->Render();
		m_chain->Present(0, 0);
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

}
