#include "Gfx.h"
#include "DX11.h"
#include "Gui.h"
#include <dxgi1_2.h>
#include <imgui/imgui.h>
#include <utils_window.h>
#include <utils_dwm.h>
// #include <gfx/widgets/ImGui/StyleEditorWidget.h>
// #include <gfx/widgets/ImGui/PluginsWidget.h>
// #include <gfx/widgets/ImGui/ControlPanelWidget.h>
// #include <gfx/widgets/ImGui/RecoverWindowsWidget.h>
// #include <gfx/widgets/ImGui/TasksWidget.h>
// #include <gfx/widgets/ImGui/DebugWidget.h>
// #include <gfx/widgets/ImGui/QuickBarWidget.h>
// #include <gfx/widgets/ImGui/TrayWidget.h>
#include <gfx/widgets/ImGui/PagerWidget.h>
#include <gfx/widgets/ImGui/MenuWidget.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <tmpl/tmpl.h>

namespace bb {
namespace imgui {

	Gfx::~Gfx () { }

	bool Gfx::Init (GfxConfig & config)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "Initializing gfx");
		m_dx11 = new DX11;
		if (m_dx11->CreateDeviceD3D(nullptr) < 0)
		{
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "Cannot create DX11 device");
			m_dx11->CleanupDeviceD3D();
			return false;
		}
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT, "Created DX11 device");
		CreateDeviceObjects();
		return CreateWidgets(config);
	}

	bool Gfx::CreateWidgets (GfxConfig & config)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
		m_config = config;

		SecondMon s = { 0 };
		EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, reinterpret_cast<LPARAM>(&s));
		int const szx = 128;
		int sx = szx;
		if (s.found)
		{
			sx = s.x1;
		}

		for (size_t i = 0, ie = config.m_startWidgets.size(); i < ie; ++i)
		{
			bbstring const & id = config.m_startWidgets[i];
			if (!id.empty())
			{
				GuiWidget * w = MkWidgetFromId(id.c_str());
				if (!w)
					TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX, "Cannot create widget with id=%ws", id.c_str());
			}
		}
		return true;
	}

	template <typename T>
	std::unique_ptr<GuiWidget> newWidget ()
	{
		return std::unique_ptr<T>(new T);
	}

	using widgetNewFnT = std::unique_ptr<GuiWidget> (*)();
	static constexpr std::pair<wchar_t const *, widgetNewFnT> const table[] = {
			{ PagerWidget::c_type, newWidget<PagerWidget> }
			, { MenuWidget::c_type, newWidget<MenuWidget> }
			
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

	using widgets = typelist<PagerWidget, MenuWidget>;

	template <typename T>
	using config_of = decltype(T::m_config);

	template<class T>
	std::unique_ptr<GuiWidget> mkNewWidget (WidgetConfig const & cfg)
	{
		std::unique_ptr<GuiWidget> w(new T(static_cast<config_of<T> const &>(cfg)));
		return std::move(w);
	}

	std::unique_ptr<GuiWidget> Gfx::MkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg)
	{
		std::unique_ptr<GuiWidget> w;
		return w;
	}

	bool Gfx::DestroyWindow (wchar_t const * widgetId)
	{
		for (GfxWindowPtr & win : m_windows)
		{
			if (win->GetName() == widgetId)
			{
					win->SetDestroy(true);
					return true;
				}
			}
		return false;
	}
// 			for (Gui::GuiWidgetPtr & w : win->m_gui->m_widgets)
// 			{
// 				if (widget == w.get())
// 				{
// 					//w.reset();
// 					return true;
// 				}
// 			}

	GuiWidget * Gfx::MkWidgetFromId (wchar_t const * widgetId)
	{
		YAML::Node & y_widgets = m_y_root["Widgets"];
		if (y_widgets)
		{
			int const n = y_widgets.size();
			for (int i = 0; i < n; ++i)
			{
				YAML::Node & y_widgets_i = y_widgets[i];
				bb::WidgetConfig cfg = y_widgets[i].as<bb::WidgetConfig>();
				if (cfg.m_id == widgetId)
				{
					std::unique_ptr<GuiWidget> w = MkWidgetFromType(cfg.m_widgetType.c_str());
					if (w && w->loadConfig(y_widgets_i))
					{
						GuiWidget * widget = MkWindowForWidget(cfg.m_x, cfg.m_y, cfg.m_w, cfg.m_h, cfg.m_alpha, std::move(w));
						return widget;
					}
				}
			}
		}
		return false;
	}

	GuiWidget * Gfx::FindWidget (wchar_t const * widgetId)
	{
		for (GfxWindowPtr & win : m_windows)
		{
			if (GuiWidget * w = win->FindWidget(widgetId))
			{
				return w;
			}
		}
		return nullptr;
	}

	HWND Gfx::MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname)
	{
		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, &Gui::GuiWndProcDispatch, 0L, 0L, ::GetModuleHandle(NULL), NULL, ::LoadCursor(NULL, IDC_ARROW), NULL, NULL, clname, NULL };
		::RegisterClassEx(&wc);

		bool const dwm_on = isDwmEnabled();
		DWORD f = 0;
		if (dwm_on)
			f |= WS_EX_LAYERED;
		HWND hwnd = ::CreateWindowExW(
			  f
			, clname
			, wname
			//, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
			, 0
			, x, y, w, h
			, NULL
			, NULL
			, wc.hInstance
			, gui);

		if (dwm_on)
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), alpha, ULW_ALPHA);
		removeWindowBorder(hwnd); // @TODO: removeWindowBorder triggers WM_PAINT but it's too early (will be ignored)

		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Created window for gui (dwm=%u), hwnd=0x%08x", dwm_on, hwnd);
		return hwnd;
	}

	GuiWidget * Gfx::MkWindowForWidget (int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_GFX);
		bool const show = widget->Visible(); // ???

		std::unique_ptr<Gui> gui(new Gui);
		gui->m_gfx = this;
		std::unique_ptr<GfxWindow> gw(new GfxWindow);
		gui->m_gfxWindow = gw.get();
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Created new gui @ 0x%x iud=%ws", gui, widget->GetId().c_str());
		HWND hwnd = MkWindow(static_cast<void *>(gui.get()), x, y, w, h, a, widget->GetWidgetTypeName(), widget->GetId().c_str());
		gw->m_hwnd = hwnd;
		gw->m_chain = m_dx11->CreateSwapChain(hwnd);
		gw->m_view = nullptr; // created in WM_SIZE @see Gui::WndProcHandler
		gw->m_clName = std::move(bbstring(widget->GetWidgetTypeName()));
		gw->m_wName = std::move(bbstring(widget->GetId()));
		if (!gw->m_gui)
		{
			gw->m_gui = std::move(gui);
			gw->m_gui->m_name = widget->GetWidgetTypeName();
			gw->m_gui->m_show = show;
			gw->m_gui->m_gfxWindow = gw.get();
			gw->m_gui->Init(gw.get());
		}
		m_newWindows.push_back(std::move(gw)); // @NOTE: new windows are waiting in m_newWindows until next frame

		// @TODO: find better place for this
		ImGuiStyle & style = ImGui::GetStyle();
		RECT rect;
		::GetClientRect(hwnd, &rect);
		int const cst = style.WindowRounding; // dumb correction
		createRoundedRect(hwnd, rect.right, rect.bottom, style.WindowRounding, cst);

		::ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
		showInFromTaskBar(hwnd, false);
		m_tasks.AddWidgetTask(m_newWindows.back().get());
		GfxWindow * win = m_newWindows.back().get();
		widget->m_gfxWindow = win;
		win->m_gui->m_widgets.push_back(std::move(widget));
		return win->m_gui->m_widgets.back().get();
	}

	bool Gfx::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Terminating gfx");
		if (m_dx11)
		{
			ReleaseDeviceObjects();
			ImGui::Shutdown();

			m_dx11->CleanupDeviceD3D();
			delete m_dx11;
			m_dx11 = nullptr;
		}
		return true;
	}

	void Gfx::NewFrame ()
	{
		for (GfxWindowPtr & w : m_newWindows)
			m_windows.push_back(std::move(w));
		m_newWindows.clear();

		for (GfxWindowPtr & w : m_windows)
		{
			w->NewFrame();
		}
	}

	void Gfx::Render ()
	{
		if (IsReady())
		{
			UpdateIconCache();
			for (GfxWindowPtr & w : m_windows)
			{
				w->Render();
			}
		}

		for (GfxWindowPtr & w : m_windows)
		{
			if (w->m_destroy)
			{
				w->Done();
				w.reset();
			}
		}

		m_windows.erase(std::remove_if(m_windows.begin(), m_windows.end(),
			[] (GfxWindowPtr const & ti_ptr) { return ti_ptr.get() == nullptr; }), m_windows.end());
	}

	bool Gfx::FindIconCoords (IconId id, void * & texture, float & u0, float & v0, float & u1, float & v1) const
	{
		bb::imgui::IconSlab const * slab = nullptr;
		if (m_iconCache.GetSlab(id, slab))
		{
			ImVec2 sz((float)id.m_size, (float)id.m_size);
			ImVec2 uv0(0.0f, 0.0f);
			ImVec2 uv1(1.0f, 1.0f);
			ImTextureID texid = nullptr;
			if (slab->Get(id.m_index, texid, uv0, uv1))
			{
				texture = texid;
				u0 = uv0.x;
				v0 = uv0.y;
				u1 = uv1.x;
				v1 = uv1.y;
				return true;
			}
		}
		return false;
	}

	void Gfx::UpdateIconCache ()
	{
		for (auto & it : m_iconCache.m_slabs)
		{
			for (auto & it2 : it.second->m_slabs)
			{
				if (it2->m_updated < it2->m_end)
				{
					if (it2->m_view == nullptr)
					{
						MkIconResourceView(*it2);
					}
					else
					{
						UpdateIconResourceView(*it2);
					}
				}
			}
		}
	}

#define USE_DYNAMIC_UPDATE 0

	bool Gfx::MkIconResourceView (IconSlab & slab)
	{
		uint8_t * bmpdata = slab.m_buffer.get();
		uint32_t const x = slab.m_x * slab.m_nx;
		uint32_t const y = slab.m_y * slab.m_ny;

		ID3D11ShaderResourceView * texview = nullptr;

		D3D11_TEXTURE2D_DESC texdesc = CD3D11_TEXTURE2D_DESC(
				DXGI_FORMAT_B8G8R8A8_UNORM
			,	x, y
			, 1, 1				// arraySize, mipLevels
			, D3D11_BIND_SHADER_RESOURCE		// bindFlags
#if USE_DYNAMIC_UPDATE
			, D3D11_USAGE_DYNAMIC						// dynamic usage. just for testing, default should be better (updates of icon textures should be rare)
			, D3D11_CPU_ACCESS_WRITE				// cpuaccessFlags
#else
			, D3D11_USAGE_DEFAULT						// default usage.
			, 0															// cpuaccessFlags
#endif
			, 1, 0, 0			// sampleCount, sampleQuality, miscFlags
			);

		ID3D11Texture2D * texture = nullptr;
		HRESULT hr = m_dx11->m_pd3dDevice->CreateTexture2D(&texdesc, nullptr, &texture);
		if (SUCCEEDED(hr))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
			memset(&viewdesc, 0, sizeof(viewdesc));
			viewdesc.Format = texdesc.Format;
			viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewdesc.Texture2D.MipLevels = 0;
			viewdesc.Texture2D.MostDetailedMip = texdesc.MipLevels;

			HRESULT hr_view = m_dx11->m_pd3dDevice->CreateShaderResourceView(texture, nullptr, &texview);

			if (SUCCEEDED(hr_view))
			{
				slab.m_view = texview;
				slab.m_updated = slab.m_end;

				UINT const srcRowPitch = slab.m_bits / CHAR_BIT * x;
				m_dx11->m_pd3dDeviceContext->UpdateSubresource(texture, 0, nullptr, bmpdata, srcRowPitch, 0); // update whole texture
			}
			texture->Release();
			return true;
		}
		return false;
	}

	bool Gfx::UpdateIconResourceView (IconSlab & slab)
	{
#if USE_DYNAMIC_UPDATE
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

		m_dx11->m_pd3dDeviceContext->Map(v->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, bmpdata, x * y * bits / CHAR_BIT);
		m_dx11->m_pd3dDeviceContext->Unmap(vertexBuffer2.Get(), 0);
#else

		uint8_t * const bmpdata = slab.m_buffer.get();

		for (; slab.m_updated < slab.m_end; ++slab.m_updated)
		{
			uint32_t const index = slab.m_updated;

			uint32_t const bytes = slab.m_bits / CHAR_BIT;
			uint32_t const row_size = bytes * slab.m_x * slab.m_nx;
			uint32_t const sz = static_cast<float>(slab.m_x);
			uint32_t const x = index % slab.m_nx;
			uint32_t const y = index / slab.m_nx;
			uint32_t const u0x = x * slab.m_x;
			uint32_t const u0y = y * slab.m_y;
			uint32_t const u1x = u0x + sz;
			uint32_t const u1y = u0y + sz;

			D3D11_BOX destRegion;
			destRegion.left = u0x;
			destRegion.top = u0y;
			destRegion.right = u1x;
			destRegion.bottom = u1y;
			destRegion.front = 0;
			destRegion.back = 1;

			ID3D11Resource * res = nullptr;
			slab.m_view->GetResource(&res);
			void * const data = bmpdata + row_size * y + x * bytes * slab.m_x;
			m_dx11->m_pd3dDeviceContext->UpdateSubresource(res, 0, &destRegion, data, row_size, 0);
		}

// update whole texture
// 		UINT const srcRowPitch = slab.m_bits / CHAR_BIT * slab.m_x * slab.m_nx;
// 		ID3D11Resource * res = nullptr;
// 		slab.m_view->GetResource(&res);
// 		m_dx11->m_pd3dDeviceContext->UpdateSubresource(res, 0, nullptr, bmpdata, srcRowPitch, 0);
		return true;
#endif
	}

	struct VERTEX_CONSTANT_BUFFER
	{
		float				 mvp[4][4];
	};


	bool Gfx::CreateDeviceObjects ()
	{
		if (m_hasDeviceObjects)
			ReleaseDeviceObjects();

		// Create the vertex shader
		{
			static const char* vertexShader = 
				"cbuffer vertexBuffer : register(b0) \
						{\
						float4x4 ProjectionMatrix; \
						};\
						struct VS_INPUT\
						{\
						float2 pos : POSITION;\
						float4 col : COLOR0;\
						float2 uv  : TEXCOORD0;\
						};\
						\
						struct PS_INPUT\
						{\
						float4 pos : SV_POSITION;\
						float4 col : COLOR0;\
						float2 uv  : TEXCOORD0;\
						};\
						\
						PS_INPUT main(VS_INPUT input)\
						{\
						PS_INPUT output;\
						output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
						output.col = input.col;\
						output.uv  = input.uv;\
						return output;\
						}";

			D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &m_pVertexShaderBlob, NULL);
			if (m_pVertexShaderBlob == NULL) // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
				return false;
			if (m_dx11->m_pd3dDevice->CreateVertexShader((DWORD*)m_pVertexShaderBlob->GetBufferPointer(), m_pVertexShaderBlob->GetBufferSize(), NULL, &m_pVertexShader) != S_OK)
				return false;

			// Create the input layout
			D3D11_INPUT_ELEMENT_DESC localLayout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,	 0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	 0, (size_t)(&((ImDrawVert*)0)->uv),	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			if (m_dx11->m_pd3dDevice->CreateInputLayout(localLayout, 3, m_pVertexShaderBlob->GetBufferPointer(), m_pVertexShaderBlob->GetBufferSize(), &m_pInputLayout) != S_OK)
				return false;

			// Create the constant buffer
			{
				D3D11_BUFFER_DESC cbDesc;
				cbDesc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
				cbDesc.Usage = D3D11_USAGE_DYNAMIC;
				cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				cbDesc.MiscFlags = 0;
				m_dx11->m_pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pVertexConstantBuffer);
			}
		}

		// Create the pixel shader
		{
			static const char* pixelShader =
				"struct PS_INPUT\
						{\
						float4 pos : SV_POSITION;\
						float4 col : COLOR0;\
						float2 uv  : TEXCOORD0;\
						};\
						sampler sampler0;\
						Texture2D texture0;\
						\
						float4 main(PS_INPUT input) : SV_Target\
						{\
						float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
						return out_col; \
						}";

			D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &m_pPixelShaderBlob, NULL);
			if (m_pPixelShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
				return false;
			if (m_dx11->m_pd3dDevice->CreatePixelShader((DWORD*)m_pPixelShaderBlob->GetBufferPointer(), m_pPixelShaderBlob->GetBufferSize(), NULL, &m_pPixelShader) != S_OK)
				return false;
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

		// Create the rasterizer state
		{
			D3D11_RASTERIZER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;
			desc.ScissorEnable = true;
			desc.DepthClipEnable = true;
			m_dx11->m_pd3dDevice->CreateRasterizerState(&desc, &m_pRasterizerState);
		}

		CreateFontsTexture();

		m_hasDeviceObjects = true;
		return true;
	}

	void Gfx::CreateFontsTexture ()
	{
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels = nullptr;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		// Upload texture to graphics system
		{
			D3D11_TEXTURE2D_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(texDesc));
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;

			ID3D11Texture2D *pTexture = NULL;
			D3D11_SUBRESOURCE_DATA subResource;
			subResource.pSysMem = pixels;
			subResource.SysMemPitch = texDesc.Width * 4;
			subResource.SysMemSlicePitch = 0;
			m_dx11->m_pd3dDevice->CreateTexture2D(&texDesc, &subResource, &pTexture);

			// Create texture view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			m_dx11->m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_pFontTextureView);
			pTexture->Release();
		}

		io.Fonts->TexID = (void *)m_pFontTextureView; // TexID == pointer to texture

																									// Create texture sampler
		{
			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.f;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MinLOD = 0.f;
			samplerDesc.MaxLOD = 0.f;
			m_dx11->m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_pFontSampler);
		}
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
			ImGui::GetIO().Fonts->TexID = 0;
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
		ImDrawData * draw_data = ImGui::GetDrawData();

		// Create and grow vertex/index buffers if needed
		if (!m_pVB || m_VertexBufferSize < draw_data->TotalVtxCount)
		{
			if (m_pVB) { m_pVB->Release(); m_pVB = NULL; }
			m_VertexBufferSize = draw_data->TotalVtxCount + 5000;
			D3D11_BUFFER_DESC desc;
			memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.ByteWidth = m_VertexBufferSize * sizeof(ImDrawVert);
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			if (m_dx11->m_pd3dDevice->CreateBuffer(&desc, NULL, &m_pVB) < 0)
				return;
		}
		if (!m_pIB || m_IndexBufferSize < draw_data->TotalIdxCount)
		{
			if (m_pIB) { m_pIB->Release(); m_pIB = NULL; }
			m_IndexBufferSize = draw_data->TotalIdxCount + 10000;
			D3D11_BUFFER_DESC bufferDesc;
			memset(&bufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = m_IndexBufferSize * sizeof(ImDrawIdx);
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			if (m_dx11->m_pd3dDevice->CreateBuffer(&bufferDesc, NULL, &m_pIB) < 0)
				return;
		}

		// Copy and convert all vertices into a single contiguous buffer
		D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
		if (m_dx11->m_pd3dDeviceContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
			return;
		if (m_dx11->m_pd3dDeviceContext->Map(m_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
			return;
		ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
		ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, &cmd_list->VtxBuffer[0], cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
			memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.size();
			idx_dst += cmd_list->IdxBuffer.size();
		}
		m_dx11->m_pd3dDeviceContext->Unmap(m_pVB, 0);
		m_dx11->m_pd3dDeviceContext->Unmap(m_pIB, 0);

		// Setup orthographic projection matrix into our constant buffer
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (m_dx11->m_pd3dDeviceContext->Map(m_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
				return;

			VERTEX_CONSTANT_BUFFER* pConstantBuffer = (VERTEX_CONSTANT_BUFFER*)mappedResource.pData;
			const float L = 0.0f;
			const float R = ImGui::GetIO().DisplaySize.x;
			const float B = ImGui::GetIO().DisplaySize.y;
			const float T = 0.0f;
			const float mvp[4][4] =
			{
				{ 2.0f/(R-L),		0.0f,						0.0f,				0.0f },
				{ 0.0f,					2.0f/(T-B),			0.0f,				0.0f },
				{ 0.0f,					0.0f,						0.5f,				0.0f },
				{ (R+L)/(L-R),	(T+B)/(B-T),		0.5f,				1.0f },
			};
			memcpy(&pConstantBuffer->mvp, mvp, sizeof(mvp));
			m_dx11->m_pd3dDeviceContext->Unmap(m_pVertexConstantBuffer, 0);
		}

		// Setup viewport
		{
			D3D11_VIEWPORT vp;
			memset(&vp, 0, sizeof(D3D11_VIEWPORT));
			vp.Width = ImGui::GetIO().DisplaySize.x;
			vp.Height = ImGui::GetIO().DisplaySize.y;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			m_dx11->m_pd3dDeviceContext->RSSetViewports(1, &vp);
		}

		// Bind shader and vertex buffers
		unsigned stride = sizeof(ImDrawVert);
		unsigned int offset = 0;
		m_dx11->m_pd3dDeviceContext->IASetInputLayout(m_pInputLayout);
		m_dx11->m_pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
		m_dx11->m_pd3dDeviceContext->IASetIndexBuffer(m_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		m_dx11->m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_dx11->m_pd3dDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
		m_dx11->m_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &m_pVertexConstantBuffer);
		m_dx11->m_pd3dDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
		m_dx11->m_pd3dDeviceContext->PSSetSamplers(0, 1, &m_pFontSampler);

		// Setup render state
		const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		m_dx11->m_pd3dDeviceContext->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);
		m_dx11->m_pd3dDeviceContext->RSSetState(m_pRasterizerState);

		// Render command lists
		int vtx_offset = 0;
		int idx_offset = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					const D3D11_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
					m_dx11->m_pd3dDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&pcmd->TextureId);
					m_dx11->m_pd3dDeviceContext->RSSetScissorRects(1, &r);
					m_dx11->m_pd3dDeviceContext->DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.size();
		}

		// Restore modified state
		m_dx11->m_pd3dDeviceContext->IASetInputLayout(NULL);
		m_dx11->m_pd3dDeviceContext->PSSetShader(NULL, NULL, 0);
		m_dx11->m_pd3dDeviceContext->VSSetShader(NULL, NULL, 0);
	}

}}
