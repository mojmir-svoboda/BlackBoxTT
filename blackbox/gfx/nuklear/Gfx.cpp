#include "Gfx.h"
#include <blackbox/gfx/shared/DX11.h>
#include "Gui.h"
#include <dxgi1_2.h>
#include <nuklear/nuklear_config.h>
#include <nuklear/nuklear_d3d11_vertex_shader.h>
#include <nuklear/nuklear_d3d11_pixel_shader.h>
#include <utils_window.h>
#include <utils_dwm.h>
#include <gfx/widgets/nuklear/PagerWidget.h>
#include <gfx/widgets/nuklear/MenuWidget.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <tmpl/tmpl.h>

namespace bb {
namespace nuklear {

	Gfx::~Gfx () { }


	template <typename T>
	std::unique_ptr<GuiWidget> newWidget ()
	{
		return std::unique_ptr<T>(new T);
	}

	using widgetNewFnT = std::unique_ptr<GuiWidget> (*)();
	static constexpr std::pair<wchar_t const *, widgetNewFnT> const table[] = {
				{ PagerWidget::c_type, newWidget<PagerWidget> }
			, { MenuWidget::c_type, newWidget<MenuWidget> }
			//, { StyleEditorWidget::c_type, newWidget<StyleEditorWidget> }
			
		//,	{}
	};

	std::unique_ptr<GuiWidget> Gfx::MkWidgetFromType (wchar_t const * widgetType)
	{
		std::unique_ptr<GuiWidget> w;
		for (auto & t : table)
			if (0 == wcscmp(t.first, widgetType))
			{
				w = t.second();
				return w;
			}

		return w;
	}

	using widgets = typelist<PagerWidget, MenuWidget/*, StyleEditorWidget*/>;

	template <typename T>
	using config_of = decltype(T::m_config);

	template<class T>
	std::unique_ptr<GuiWidget> mkNewWidget (WidgetConfig const & cfg)
	{
		std::unique_ptr<GuiWidget> w(new T(static_cast<config_of<T> const &>(cfg)));
		return std::move(w);
	}

	template<int... Is> struct seq { };
	template<int N, int... Is> struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };
	template<int... Is> struct gen_seq<0, Is...> : seq<Is...> { };
	template<class L>
	struct tmpl_sizeof_impl;
	template<template<class...> class L, class... T>
	struct tmpl_sizeof_impl<L<T...>>
	{
		using type = std::integral_constant<std::size_t, sizeof...(T)>;
	};
	template<class L>
	using tmpl_size = typename tmpl_sizeof_impl<L>::type;

	template<size_t Idx>
	using idx2type = tmpl_at_c<widgets, Idx>;

	template<int N>
	std::unique_ptr<GuiWidget> mkNth (WidgetConfig const & cfg) { return mkNewWidget<idx2type<N>>(cfg); }
	template<int... Ns>
	std::unique_ptr<GuiWidget> mkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg, seq<Ns...>)
	{
		using mk_fn_prototype = std::unique_ptr<GuiWidget> (*) (WidgetConfig const & cfg);
		constexpr static wchar_t const * const names[] = { idx2type<Ns>::c_type... };
		constexpr static mk_fn_prototype const funcs[] = { &mkNth<Ns>... };
		constexpr size_t const sz = sizeof...(Ns);

		for (size_t i = 0; i < sz; ++i)
		{
			if (0 == wcscmp(names[i], widgetType))
				return (*funcs[i])(cfg);
		}
		return std::unique_ptr<GuiWidget>();
	}
	inline std::unique_ptr<GuiWidget> mkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg)
	{
		return mkWidgetFromType(widgetType, cfg, gen_seq<tmpl_size<widgets>::value>{ });
	}

	std::unique_ptr<GuiWidget> Gfx::MkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg)
	{
		return mkWidgetFromType(widgetType, cfg);
	}

	GuiWidget * Gfx::MkWindowForWidget(int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_GFX);
		bool const show = widget->Visible(); // ???

		std::unique_ptr<Gui> gui(new Gui);
		Gui * nk_gui = gui.get();
		gui->m_gfx = this;
		std::unique_ptr<shared::GfxWindow> gw(new shared::GfxWindow);
		gui->m_gfxWindow = gw.get();
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Created new gui @ 0x%x iud=%ws", gui.get(), widget->GetId().c_str());
		HWND hwnd = MkWindow(static_cast<void *>(gui.get()), x, y, w, h, a, widget->GetWidgetTypeName(), widget->GetId().c_str());
		gw->m_hwnd = hwnd;
		gw->m_chain = m_dx11->CreateSwapChain(hwnd);
		gw->m_view = nullptr; // created in WM_SIZE @see Gui::WndProcHandler
		gw->m_clName = std::move(bbstring(widget->GetWidgetTypeName()));
		gw->m_wName = std::move(bbstring(widget->GetId()));
		if (!gw->m_gui)
		{
			nk_gui->m_gfxWindow = gw.get();
			gw->m_gui = std::move(gui);
			gw->m_gui->m_name = widget->GetWidgetTypeName();
			gw->m_gui->m_show = show;
			gw->m_gui->Init(gw.get());
		}
		m_newWindows.push_back(std::move(gw)); // @NOTE: new windows are waiting in m_newWindows until next frame

		// @TODO: find better place for this
		//ImGuiStyle & style = ImGui::GetStyle();
		RECT rect;
		::GetClientRect(hwnd, &rect);
		//int const cst = 20; // dumb correction
		createRoundedRect(hwnd, rect.right, rect.bottom, 20, 0);

		showInFromTaskBar(hwnd, false);
		m_tasks.AddWidgetTask(m_newWindows.back().get());
		GfxWindow * win = m_newWindows.back().get();
		widget->m_gfxWindow = win;
		nk_gui->m_widgets.push_back(std::move(widget));
		return nk_gui->m_widgets.back().get();
	}

	bool Gfx::Done ()
	{
		shared::Gfx::Done();
		// @TODO: done nk
		return true;
	}

	void Gfx::ResetInput ()
	{
		for (auto & it : m_windows)
		{
			it->GetGui()->ResetInput();
		}
	}

	bool Gfx::CreateDeviceObjects ()
	{
		if (m_hasDeviceObjects)
			ReleaseDeviceObjects();

			/* vertex shader */
			HRESULT hr = m_dx11->m_pd3dDevice->CreateVertexShader(nk_d3d11_vertex_shader, sizeof(nk_d3d11_vertex_shader), NULL, &m_pVertexShader);
			//NK_ASSERT(SUCCEEDED(hr));

			// Create the input layout
			/* input layout */
			{
				const D3D11_INPUT_ELEMENT_DESC layout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(struct nk_d3d11_vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(struct nk_d3d11_vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, offsetof(struct nk_d3d11_vertex, col),      D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			hr = m_dx11->m_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), nk_d3d11_vertex_shader, sizeof(nk_d3d11_vertex_shader), &m_pInputLayout);
			//NK_ASSERT(SUCCEEDED(hr));
		}

		/* pixel shader */
		{
			HRESULT hr = m_dx11->m_pd3dDevice->CreatePixelShader(nk_d3d11_pixel_shader, sizeof(nk_d3d11_pixel_shader), NULL, &m_pPixelShader);
			//NK_ASSERT(SUCCEEDED(hr)); 
		}

		// Create the blending setup
		{
			D3D11_BLEND_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.AlphaToCoverageEnable = false;
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			m_dx11->m_pd3dDevice->CreateBlendState(&desc, &m_pBlendState);
		}

		/* vertex buffer */
		{
			D3D11_BUFFER_DESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.ByteWidth = m_VertexBufferSize;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			HRESULT hr = m_dx11->m_pd3dDevice->CreateBuffer(&desc, NULL, &m_pVB);
			//NK_ASSERT(SUCCEEDED(hr));
		}

		/* index buffer */
		{
			D3D11_BUFFER_DESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.ByteWidth = m_IndexBufferSize;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			HRESULT hr = m_dx11->m_pd3dDevice->CreateBuffer(&desc, NULL, &m_pIB);
			//NK_ASSERT(SUCCEEDED(hr));
		}

		/* sampler state */
		{
			D3D11_SAMPLER_DESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.MipLODBias = 0.0f;
			desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			desc.MinLOD = 0.0f;
			desc.MaxLOD = FLT_MAX;
			HRESULT hr = m_dx11->m_pd3dDevice->CreateSamplerState(&desc, &m_pFontSampler);
			//NK_ASSERT(SUCCEEDED(hr));
		}

		CreateFontsTexture();

		m_hasDeviceObjects = true;
		return true;
	}

	void Gfx::CreateFontsTexture ()
	{
	}

	void Gfx::ReleaseDeviceObjects ()
	{
		if (!m_dx11)
			return;

		if (m_pFontSampler)
		{
			m_pFontSampler->Release();
			m_pFontSampler = NULL;
		}
		if (m_pFontTextureView)
		{
			m_pFontTextureView->Release();
			m_pFontTextureView = NULL;
			//ImGui::GetIO().Fonts->TexID = 0;
		}
		if (m_pIB)
		{
			m_pIB->Release();
			m_pIB = NULL;
		}
		if (m_pVB)
		{
			m_pVB->Release();
			m_pVB = NULL;
		}

		if (m_pBlendState)
		{
			m_pBlendState->Release();
			m_pBlendState = NULL;
		}
		if (m_pRasterizerState)
		{
			m_pRasterizerState->Release();
			m_pRasterizerState = NULL;
		}
		if (m_pPixelShader)
		{
			m_pPixelShader->Release();
			m_pPixelShader = NULL;
		}
		if (m_pPixelShaderBlob)
		{
			m_pPixelShaderBlob->Release();
			m_pPixelShaderBlob = NULL;
		}
		if (m_pVertexConstantBuffer)
		{
			m_pVertexConstantBuffer->Release();
			m_pVertexConstantBuffer = NULL;
		}
		if (m_pInputLayout)
		{
			m_pInputLayout->Release();
			m_pInputLayout = NULL;
		}
		if (m_pVertexShader)
		{
			m_pVertexShader->Release();
			m_pVertexShader = NULL;
		}
		if (m_pVertexShaderBlob)
		{
			m_pVertexShaderBlob->Release();
			m_pVertexShaderBlob = NULL;
		}

		m_hasDeviceObjects = false;
	}

	void Gfx::DeviceLost ()
	{
		ReleaseDeviceObjects();
		CreateDeviceObjects();
	}

	void Gfx::RenderImGui ()
	{
		//nk_d3d11_render(this, m_dx11->m_pd3dDeviceContext, NK_ANTI_ALIASING_ON);
	}

}}
