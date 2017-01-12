#pragma once
#include "platform_win.h"
#include "imgui.h"
#include <bblib/bbstring.h>
#include <blackbox/gfx/GfxWindow.h>
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

namespace bb {
namespace imgui {
	struct Gui;

	struct GfxWindow : ::bb::GfxWindow
	{
		IDXGISwapChain1 * m_chain { nullptr };
		ID3D11RenderTargetView * m_view { nullptr };
		ImVec4 m_clrCol { ImColor(0, 0, 0, 255) };

		GfxWindow () { }
		virtual ~GfxWindow () { }
		virtual void Render () override;
		virtual void NewFrame () override;
		virtual bool Done () override;
		virtual void Show (bool on) override;
		virtual bool Visible () const override;
		virtual void DrawUI () { }
	};
}}

