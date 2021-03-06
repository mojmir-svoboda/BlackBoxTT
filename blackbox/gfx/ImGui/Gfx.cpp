#include "Gfx.h"
#include <blackbox/gfx/shared/DX11.h>
#include "Gui.h"
#include <blackbox/gfx/shared/GfxWindow.h>
#include <dxgi1_2.h>
#include <imgui/imgui.h>
#include <utils_window.h>
#include <utils_dwm.h>
#include <gfx/widgets/ImGui/StyleEditorWidget.h>
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


	template <typename T>
	std::unique_ptr<GuiWidget> newWidget ()
	{
		return std::unique_ptr<T>(new T);
	}

	using widgetNewFnT = std::unique_ptr<GuiWidget> (*)();
	static constexpr std::pair<wchar_t const *, widgetNewFnT> const table[] = {
				{ PagerWidget::c_type, newWidget<PagerWidget> }
			, { MenuWidget::c_type, newWidget<MenuWidget> }
			, { StyleEditorWidget::c_type, newWidget<StyleEditorWidget> }
			
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

	using widgets = typelist<PagerWidget, MenuWidget, StyleEditorWidget>;

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

	GuiWidget * Gfx::MkWindowForWidget (int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_GFX);
		bool const show = widget->Visible(); // ???

		std::unique_ptr<Gui> gui(new Gui);
		Gui * imgui_gui = gui.get();
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
			gw->m_gui = std::move(gui);
			gw->m_gui->m_name = widget->GetWidgetTypeName();
			gw->m_gui->m_show = show;
			imgui_gui->m_gfxWindow = gw.get();
			gw->m_gui->Init(gw.get());
		}
		m_newWindows.push_back(std::move(gw)); // @NOTE: new windows are waiting in m_newWindows until next frame

																					 // @TODO: find better place for this
		ImGuiStyle & style = ImGui::GetStyle();
		RECT rect;
		::GetClientRect(hwnd, &rect);
		int const cst = style.WindowRounding; // dumb correction
		createRoundedRect(hwnd, rect.right, rect.bottom, style.WindowRounding, cst);

		showInFromTaskBar(hwnd, false);
		m_tasks.AddWidgetTask(m_newWindows.back().get());
		GfxWindow * win = m_newWindows.back().get();
		widget->m_gfxWindow = win;
		imgui_gui->m_widgets.push_back(std::move(widget));
		return imgui_gui->m_widgets.back().get();
	}

	bool Gfx::Done ()
	{
		if (m_dx11)
			ImGui::Shutdown();
		shared::Gfx::Done();
		return true;
	}

	struct VERTEX_CONSTANT_BUFFER
	{
		float				 mvp[4][4];
	};

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


}}
