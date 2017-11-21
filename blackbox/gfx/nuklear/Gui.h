#pragma once
#include <nuklear/nuklear_config.h>
#include <platform_win.h>
#include <vector>
#include <blackbox/gfx/Gui.h>
#include <blackbox/gfx/GuiWidget.h>
#include <bblib/bbstring.h>
struct ID3D11Buffer; struct ID3D10Blob;
struct ID3D11VertexShader; struct ID3D11InputLayout; struct ID3D11PixelShader; struct ID3D11SamplerState;
struct ID3D11ShaderResourceView; struct ID3D11RasterizerState; struct ID3D11BlendState;

namespace bb {
namespace shared {
	struct GfxWindow;
}
namespace nuklear {
	struct Gfx;

	struct Gui : bb::Gui
	{
		Gui () { }
		HWND m_hwnd { nullptr };
		Gfx * m_gfx { nullptr };
		shared::GfxWindow * m_gfxWindow { nullptr };
		nk_context m_context { };

		ID3D11Buffer *						m_pVB{ nullptr };
		ID3D11Buffer *						m_pIB{ nullptr };
		ID3D10Blob *							m_pVertexShaderBlob{ nullptr };
		ID3D11VertexShader *			m_pVertexShader{ nullptr };
		ID3D11InputLayout *				m_pInputLayout{ nullptr };
		ID3D11Buffer *						m_pVertexConstantBuffer{ nullptr };
		ID3D10Blob *							m_pPixelShaderBlob{ nullptr };
		ID3D11PixelShader *				m_pPixelShader{ nullptr };
		ID3D11SamplerState *			m_pFontSampler{ nullptr };
		ID3D11ShaderResourceView * m_pFontTextureView{ nullptr };
		ID3D11RasterizerState *		m_pRasterizerState{ nullptr };
		ID3D11BlendState *				m_pBlendState{ nullptr };

		using GuiWidgetPtr = std::unique_ptr<GuiWidget>;
		std::vector<GuiWidgetPtr> m_widgets;
		bbstring m_name { };

		virtual void ResetInput () override;
		virtual void NewFrame () override;
		virtual void DrawUI () override;
		virtual void Render () override;
		virtual bool Init (bb::GfxWindow * w) override;
		virtual bool Done () override;

		virtual void Show (bool on) { m_show = on; }
		virtual bool Visible () const { return m_show; }
		virtual GuiWidget * FindWidget (wchar_t const * widgetId) override;
		virtual bool RmWidget (GuiWidget * widget) override;

		virtual LRESULT WndProcHandler (HWND, UINT msg, WPARAM wParam, LPARAM lParam) override;

		void OnResize (unsigned w, unsigned h);
		void FeedInput ();
	};
}}

