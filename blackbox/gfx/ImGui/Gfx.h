#pragma once
#include "platform_win.h"
#include "DX11.h"
#include <vector>
#include "Gui.h"
#include "IconCache.h"
#include "GfxWindow.h"
#include <blackbox/gfx/Gfx.h>
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/WidgetConfig.h>
#include <blackbox/GfxConfig.h>
#include <blackbox/Tasks.h>
namespace YAML { class Node; }

namespace bb {
namespace imgui {

	struct Gfx : bb::Gfx
	{
		Tasks & m_tasks;
		YAML::Node & m_y_root;
		GfxConfig m_config;
		DX11 * m_dx11 { nullptr };
		std::vector<ID3D11Texture2D *> m_textures;
		using GfxWindowPtr = std::unique_ptr<GfxWindow>;
		std::vector<GfxWindowPtr> m_windows;
		std::vector<GfxWindowPtr> m_newWindows;
		IconCache m_iconCache;
		bool m_hasDeviceObjects { false };
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

		Gfx (Tasks & t, YAML::Node & y_root) : m_tasks(t), m_y_root(y_root) { }
		virtual ~Gfx ();
		virtual bool Init (GfxConfig & config) override;
		bool CreateWidgets (GfxConfig & config);
		bool IsReady () const { return m_dx11 != nullptr; }
		void DeviceLost ();
		bool CreateDeviceObjects ();
		void ReleaseDeviceObjects ();
		void CreateFontsTexture ();
		void RenderImGui ();

		virtual void Render () override;
		virtual void NewFrame () override;
		virtual bool Done () override;

		GfxWindow * MkWidgetWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show);
		HWND MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname);

		GfxWindow * GetGfxWindow (size_t n) { return m_windows[n].get(); }
		GfxWindow const * GetGfxWindow (size_t n) const { return m_windows[n].get(); }
		//void OnResize (unsigned w, unsigned h);

		virtual bool AddIconToCache (bbstring const & name, HICON ico, IconId & id) override
		{
			return m_iconCache.Add(name, ico, id);
		}
		virtual bool FindIconInCache (bbstring const & name, IconId & id) const override
		{
			return m_iconCache.Find(name, id);
		}
		virtual bool FindIconCoords (IconId id, void * & texture, float & u0, float & v0, float & u1, float & v1) const override;
		bool MkIconResourceView (IconSlab & slab);
		bool UpdateIconResourceView (IconSlab & slab);
		void UpdateIconCache ();

		virtual GuiWidget * FindWidget (wchar_t const * widgetId) override;
		std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType);
		virtual bool MkWidgetFromId (wchar_t const * widgetId) override;
		virtual bool DestroyWindow (wchar_t const * widgetId) override;
	};

}}

