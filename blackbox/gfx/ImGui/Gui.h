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
	struct DX11;
namespace imgui {
	struct GfxWindow;

	struct Gui : bb::Gui
	{
		Gui () { }
		bool m_hasDeviceObjects { false };
		HWND m_hwnd { nullptr };
		DX11 * m_dx11 { nullptr };
		GfxWindow * m_gfxWindow { nullptr };
		ImGuiContext * m_context { nullptr };
		std::vector<GuiWidget *> m_widgets;
		bbstring m_name { };
		ID3D11Buffer *						m_pVB { nullptr };
		ID3D11Buffer *						m_pIB { nullptr };
		ID3D10Blob *							m_pVertexShaderBlob { nullptr };
		ID3D11VertexShader *			m_pVertexShader { nullptr };
		ID3D11InputLayout *				m_pInputLayout { nullptr };
		ID3D11Buffer *						m_pVertexConstantBuffer { nullptr };
		ID3D10Blob *							m_pPixelShaderBlob { nullptr };
		ID3D11PixelShader *				m_pPixelShader { nullptr };
		ID3D11SamplerState *			m_pFontSampler { nullptr };
		ID3D11ShaderResourceView * m_pFontTextureView { nullptr };
		ID3D11RasterizerState *		m_pRasterizerState { nullptr };
		ID3D11BlendState *				m_pBlendState { nullptr };
		int												m_VertexBufferSize { 5000 };
		int												m_IndexBufferSize { 10000 };

		void AddWidget (GuiWidget * win);

		virtual void NewFrame () override;
		virtual void DrawUI () override;
		virtual void Render () override;
		virtual bool Init (bb::GfxWindow * w) override;
		virtual bool Done () override;

		virtual void Show (bool on) { m_show = on; }
		virtual bool Visible () const { return m_show; }

		bool InitDX (GfxWindow * w, DX11 * dx11);
		void RenderImGui ();

		static LRESULT CALLBACK GuiWndProcDispatch (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		LRESULT WndProcHandler (HWND, UINT msg, WPARAM wParam, LPARAM lParam);

		void OnResize (unsigned w, unsigned h);
		void FeedInput ();
		void DeviceLost ();
		bool CreateDeviceObjects ();
		void ReleaseDeviceObjects ();
		void CreateFontsTexture ();
	};
}}

