#pragma once
#include <imgui/imgui.h>
#include <platform_win.h>
#include <vector>
#include <blackbox/gfx/Gui.h>
#include <blackbox/gfx/GuiWidget.h>
#include <bblib/bbstring.h>
struct ID3D11Buffer; struct ID3D10Blob;
struct ID3D11VertexShader; struct ID3D11InputLayout; struct ID3D11PixelShader; struct ID3D11SamplerState;
struct ID3D11ShaderResourceView; struct ID3D11RasterizerState; struct ID3D11BlendState;

namespace bb {
namespace imgui {
	struct Gfx;
	struct GfxWindow;

	struct Gui : bb::Gui
	{
		Gui () { }
		HWND m_hwnd { nullptr };
		Gfx * m_gfx { nullptr };
		GfxWindow * m_gfxWindow { nullptr };
		ImGuiContext * m_context { nullptr };
		using GuiWidgetPtr = std::unique_ptr<GuiWidget>;
		std::vector<GuiWidgetPtr> m_widgets;
		bbstring m_name { };

		virtual void NewFrame () override;
		virtual void DrawUI () override;
		virtual void Render () override;
		virtual bool Init (bb::GfxWindow * w) override;
		virtual bool Done () override;

		virtual void Show (bool on) { m_show = on; }
		virtual bool Visible () const { return m_show; }
		virtual GuiWidget * FindWidget (wchar_t const * widgetId) override;
		virtual bool RmWidget (GuiWidget * widget) override;

		static LRESULT CALLBACK GuiWndProcDispatch (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		LRESULT WndProcHandler (HWND, UINT msg, WPARAM wParam, LPARAM lParam);

		void OnResize (unsigned w, unsigned h);
		void FeedInput ();
	};
}}

