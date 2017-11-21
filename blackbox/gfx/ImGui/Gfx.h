#pragma once
#include "platform_win.h"
#include <vector>
#include "Gui.h"
#include <blackbox/gfx/Gfx.h>
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/gfx/GfxWindow.h>
#include <blackbox/gfx/shared/DX11.h>
#include <blackbox/gfx/shared/Gfx.h>
//#include "IconCache.h"
#include <blackbox/WidgetConfig.h>
#include <blackbox/GfxConfig.h>
#include <blackbox/Tasks.h>
namespace YAML { class Node; }

namespace bb {
namespace imgui {

	struct Gfx : bb::shared::Gfx
	{
		bool m_hasDeviceObjects{ false };
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
		int												m_VertexBufferSize { 5000 };
		int												m_IndexBufferSize { 10000 };

		Gfx (Tasks & t, YAML::Node & y_root) : bb::shared::Gfx(t, y_root) { }
		virtual ~Gfx ();
		virtual bool Done () override;
		virtual void ResetInput () override { }
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType) override;
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg) override;
		virtual GuiWidget * MkWindowForWidget (int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget) override;
		virtual bool CreateDeviceObjects () override;
		virtual void ReleaseDeviceObjects () override;
		virtual void CreateFontsTexture () override;
		virtual void RenderImGui () override;
	};

}}

