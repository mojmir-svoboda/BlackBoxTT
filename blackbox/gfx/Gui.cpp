#include "Gui.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "DX11.h"
#include "BlackBox.h"
#include "utils_imgui.h"
#include <bblib/codecvt.h>
#include <logging.h>
#include <bblib/ScopeGuard.h>
#include <functional>

namespace bb
{
	void Gui::OnResize (unsigned w, unsigned h)
	{
		if (m_dx11->m_pd3dDevice != NULL)
		{
			ReleaseDeviceObjects();

			m_gfxWindow->m_chain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
			m_gfxWindow->m_view = m_dx11->CreateRenderTarget(m_gfxWindow->m_chain);

			CreateDeviceObjects();
		}
	}

	void Gui::Render ()
	{
		if (m_enabled)
		{
			ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
			ImGui::SetCurrentContext(m_context);
			scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
			{
				DrawUI();
				ImGui::Render();
				RenderImGui();
			}
		}
	}

	void Gui::AddWidget (GuiWidget * win)
	{
		m_widgets.push_back(win);
	}

	void Gui::DrawUI ()
	{
		if (!m_enabled)
			return;

		for (GuiWidget * w : m_widgets)
		{
			if (w->m_config.m_show)
				w->DrawUI();
		}
	}

	void Gui::FeedInput ()
	{
		ImGuiIO & io = ImGui::GetIO();

		// Read keyboard modifiers inputs
		io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
		//...
	}

	void Gui::NewFrame ()
	{
		if (m_enabled)
		{
			ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
			ImGui::SetCurrentContext(m_context);
			scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
			{
				if (!m_hasDeviceObjects)
					CreateDeviceObjects();

				ImGuiIO & io = ImGui::GetIO();

				RECT rect;
				GetClientRect(m_hwnd, &rect); // Setup display size (every frame to accommodate for window resizing)
				io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

				FeedInput();

				ImGui::NewFrame(); // Start the frame
			}
		}
	}

	LRESULT CALLBACK Gui::GuiWndProcDispatch (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		Gui * gui = nullptr;

		if (msg == WM_NCCREATE)
		{
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
			gui = reinterpret_cast<Gui *>(lpcs->lpCreateParams);
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(gui));
			//assert(gui->m_hwnd == gui->SetHWND(hwnd);
		}
		else
		{
			gui = reinterpret_cast<Gui *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (gui)
			return gui->WndProcHandler(hwnd, msg, wparam, lparam);
		else
			return DefWindowProc(hwnd, msg, wparam, lparam);;
	}

	LRESULT Gui::WndProcHandler (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(m_context);
		scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
		{
			ImGuiIO & io = ImGui::GetIO();
			switch (msg)
			{
				case WM_LBUTTONDOWN:
				{
					if (wParam & MK_CONTROL)
					{
						UpdateWindow(hwnd);
						SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
						return 0;
					}

					io.MouseDown[0] = true;
					return 0;
				}
				case WM_LBUTTONUP:
				{
					io.MouseDown[0] = false;
					return true;
				}
				case WM_RBUTTONDOWN:
				{
					io.MouseDown[1] = true;
					return true;
				}
				case WM_RBUTTONUP:
				{
					io.MouseDown[1] = false;
					return true;
				}
				case WM_MBUTTONDOWN:
				{
					if (wParam & MK_CONTROL)
					{
						UpdateWindow(hwnd);
						SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | HTCAPTION, 0);
						return 0;
					}

					io.MouseDown[2] = true;
					return true;
				}
				case WM_MBUTTONUP:
				{
					io.MouseDown[2] = false;
					return true;
				}
				case WM_MOUSEWHEEL:
				{
					io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
					return true;
				}
				case WM_MOUSEMOVE:
				{
					io.MousePos.x = (signed short)(lParam);
					io.MousePos.y = (signed short)(lParam >> 16);
					return true;
				}
				case WM_KEYDOWN:
				{
					if (wParam < 256)
						io.KeysDown[wParam] = 1;
					return true;
				}
				case WM_KEYUP:
				{
					if (wParam < 256)
							io.KeysDown[wParam] = 0;
					return true;
				}
				case WM_CHAR:
				{
					// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
					if (wParam > 0 && wParam < 0x10000)
							io.AddInputCharacter((unsigned short)wParam);
					return true;
				}
			}

			switch (msg)
			{
				case WM_SIZE:
				{
					if (m_dx11 && wParam != SIZE_MINIMIZED)
					{
						unsigned const x = (UINT)LOWORD(lParam);
						unsigned const y = (UINT)HIWORD(lParam);
						TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT, "WM_SIZE for hwnd=0x%x gui=0x%x dim=(%u,%u)", hwnd, this, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
						OnResize(x, y);
					}
					return 0;
				}
				case WM_SYSCOMMAND:
				{
					if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
					{
						return 0;
					}
					break;
				}
				case WM_DESTROY:
				{
					PostQuitMessage(0);
					return 0;
				}
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}


	bool Gui::Init (HWND hwnd, DX11 * dx11)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT , "Initializing GUI for hwnd=0x%x", hwnd);
		m_hwnd = hwnd;
		m_dx11 = dx11;
		ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Old context = 0x%x", old_ctx);
		m_context = ImGui::CreateContext();
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "New context = 0x%x", m_context);
		ImGui::SetCurrentContext(m_context);
		scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
		{
			ImGuiIO & io = ImGui::GetIO();
			io.RenderDrawListsFn = nullptr;
			io.ImeWindowHandle = m_hwnd;

			io.KeyMap[ImGuiKey_Tab] = VK_TAB;												// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
			io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			io.KeyMap[ImGuiKey_Home] = VK_HOME;
			io.KeyMap[ImGuiKey_End] = VK_END;
			io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			io.KeyMap[ImGuiKey_A] = 'A';
			io.KeyMap[ImGuiKey_C] = 'C';
			io.KeyMap[ImGuiKey_V] = 'V';
			io.KeyMap[ImGuiKey_X] = 'X';
			io.KeyMap[ImGuiKey_Y] = 'Y';
			io.KeyMap[ImGuiKey_Z] = 'Z';
		}
		return true;
	}

	struct VERTEX_CONSTANT_BUFFER
	{
		float				 mvp[4][4];
	};


	bool Gui::CreateDeviceObjects ()
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

	void Gui::CreateFontsTexture ()
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

	void Gui::ReleaseDeviceObjects ()
	{
		if (!m_dx11)
				return;

		if (m_pFontSampler) { m_pFontSampler->Release(); m_pFontSampler = NULL; }
		if (m_pFontTextureView) { m_pFontTextureView->Release(); m_pFontTextureView = NULL; ImGui::GetIO().Fonts->TexID = 0; }
		if (m_pIB) { m_pIB->Release(); m_pIB = NULL; }
		if (m_pVB) { m_pVB->Release(); m_pVB = NULL; }

		if (m_pBlendState) { m_pBlendState->Release(); m_pBlendState = NULL; }
		if (m_pRasterizerState) { m_pRasterizerState->Release(); m_pRasterizerState = NULL; }
		if (m_pPixelShader) { m_pPixelShader->Release(); m_pPixelShader = NULL; }
		if (m_pPixelShaderBlob) { m_pPixelShaderBlob->Release(); m_pPixelShaderBlob = NULL; }
		if (m_pVertexConstantBuffer) { m_pVertexConstantBuffer->Release(); m_pVertexConstantBuffer = NULL; }
		if (m_pInputLayout) { m_pInputLayout->Release(); m_pInputLayout = NULL; }
		if (m_pVertexShader) { m_pVertexShader->Release(); m_pVertexShader = NULL; }
		if (m_pVertexShaderBlob) { m_pVertexShaderBlob->Release(); m_pVertexShaderBlob = NULL; }

		m_hasDeviceObjects = false;
	}

	void Gui::DeviceLost ()
	{
		ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(m_context);
		scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
		{
			ReleaseDeviceObjects();
			CreateDeviceObjects();
		}
	}

	void Gui::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Terminating GUI");
		ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(m_context);
		scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
		{
			ReleaseDeviceObjects();
			ImGui::Shutdown();
		}

		ImGui::DestroyContext(m_context);
		m_context = nullptr;
	}

	void Gui::RenderImGui ()
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
}


