#pragma once
#include "platform_win.h"
#include <memory>
#include <bblib/bbstring.h>
#include <blackbox/gfx/Gui.h>
#include <blackbox/gfx/GfxWindow.h>
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

namespace bb {
namespace shared {

	struct GfxWindow : ::bb::GfxWindow
	{
		IDXGISwapChain1 * m_chain { nullptr };
		std::unique_ptr<Gui> m_gui { nullptr };
		ID3D11RenderTargetView * m_view { nullptr };
		float m_clrCol[4] { 0.0f, 0.0f, 0.0f, 255.0f };

		GfxWindow () { }
		virtual ~GfxWindow () { }
		virtual void Render () override;
		virtual void NewFrame () override;
		virtual bool Done () override;
		virtual void Show (bool on) override;
		virtual bool Visible () const override;
		virtual void DrawUI () { }

		virtual Gui * GetGui () override { return m_gui.get(); }
		virtual Gui const * GetGui () const override { return m_gui.get(); }

		virtual GuiWidget * FindWidget (wchar_t const * widgetId) override { return m_gui->FindWidget(widgetId); }

		bool IsReady () const { return m_chain != nullptr; }
	};
}}

