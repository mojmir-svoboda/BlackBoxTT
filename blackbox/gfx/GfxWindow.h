#pragma once
#include "platform_win.h"
#include "imgui.h"
#include <bblib/bbstring.h>
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

namespace bb {
	struct Gui;

	struct GfxWindow
	{
		HWND m_hwnd { nullptr };
		Gui * m_gui { nullptr };
		IDXGISwapChain1 * m_chain { nullptr };
		ID3D11RenderTargetView * m_view { nullptr };
		ImVec4 m_clrCol { ImColor(0, 0, 0, 255) };
		bbstring m_clName { };
		bbstring m_wName { };

		GfxWindow () { }
		virtual ~GfxWindow () { }
		void Render ();
		void NewFrame ();
		bool Done ();
		virtual void DrawUI () { }
		Gui * GetGui () { return m_gui; }
		Gui const * GetGui () const { return m_gui; }
		bbstring const & GetName () const { return m_wName; }
	};
}

