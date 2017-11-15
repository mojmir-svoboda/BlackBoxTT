#include "Gui.h"
#include <blackbox/gfx/shared/GfxWindow.h>
#include <nuklear/nuklear_config.h>
#include "Gfx.h"
#include <blackbox/gfx/shared/DX11.h>
#include "BlackBox.h"
#include "utils_imgui.h"
#include <bblib/codecvt.h>
#include <logging.h>
#include <bblib/ScopeGuard.h>
#include <functional>
#include <nuklear/nuklear_config.h>

namespace bb {
namespace nuklear {

	void Gui::OnResize (unsigned w, unsigned h)
	{
		if (m_gfxWindow->IsReady())
		{
			if (m_gfxWindow->m_view)
			{
				m_gfxWindow->m_view->Release();
				m_gfxWindow->m_view = nullptr;
			}
			m_gfx->m_dx11->m_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
			if (!SUCCEEDED(m_gfxWindow->m_chain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0)))
			{
			}

			if (m_gfx->m_pVertexConstantBuffer)
			{
				m_gfx->m_pVertexConstantBuffer->Release();
				m_gfx->m_pVertexConstantBuffer = nullptr;
			}

			/* constant buffer */
			{
				float matrix[4 * 4];
				D3D11_BUFFER_DESC desc;
				memset(&desc, 0, sizeof(desc));
				desc.ByteWidth = sizeof(matrix);
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				desc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA data;
				data.pSysMem = matrix;
				data.SysMemPitch = 0;
				data.SysMemSlicePitch = 0;

				m_gfx->m_dx11->m_pd3dDevice->CreateBuffer(&desc, &data, &m_gfx->m_pVertexConstantBuffer);

				D3D11_MAPPED_SUBRESOURCE mapped;
				if (SUCCEEDED(m_gfx->m_dx11->m_pd3dDeviceContext->Map((ID3D11Resource *)m_gfx->m_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
				{
					nk_d3d11_get_projection_matrix(w, h, (float *)mapped.pData);
					m_gfx->m_dx11->m_pd3dDeviceContext->Unmap((ID3D11Resource *)m_gfx->m_pVertexConstantBuffer, 0);
				}
			}

			/* viewport */
			{
				m_gfx->m_viewport.TopLeftX = 0.0f;
				m_gfx->m_viewport.TopLeftY = 0.0f;
				m_gfx->m_viewport.Width = (float)w;
				m_gfx->m_viewport.Height = (float)h;
				m_gfx->m_viewport.MinDepth = 0.0f;
				m_gfx->m_viewport.MaxDepth = 1.0f;
			}

			m_gfxWindow->m_view = m_gfx->m_dx11->CreateRenderTarget(m_gfxWindow->m_chain);
		}
	}

	GuiWidget * Gui::FindWidget (wchar_t const * widgetId)
	{
		for (GuiWidgetPtr & w : m_widgets)
			if (widgetId == w->GetId())
				return w.get();
		return nullptr;
	}

	bool Gui::RmWidget (GuiWidget * widget)
	{
		// TODO
		return 123;
	}

	void Gui::Render ()
	{
		if (m_show)
		{
			DrawUI();

			m_gfx->m_dx11->m_pd3dDeviceContext->ClearRenderTargetView(m_gfxWindow->m_view, (float*)&m_gfxWindow->m_clrCol);
			m_gfx->m_dx11->m_pd3dDeviceContext->OMSetRenderTargets(1, &m_gfxWindow->m_view, nullptr);
			m_gfx->RenderImGui();
		}
	}

	void Gui::DrawUI ()
	{
		if (!m_show)
			return;

		for (std::unique_ptr<GuiWidget> & w : m_widgets)
		{
			if (w->Visible())
				w->DrawUI();
		}
	}

	void Gui::FeedInput ()
	{
// 		ImGuiIO & io = ImGui::GetIO();
// 
// 		// Read keyboard modifiers inputs
// 		io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
// 		io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
// 		io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
// 		//...
	}

	void Gui::NewFrame ()
	{
		if (m_show)
		{
// 			ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
// 			ImGui::SetCurrentContext(m_context);
// 			scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
// 			{
// 				ImGuiIO & io = ImGui::GetIO();
// 
// 				RECT rect;
// 				GetClientRect(m_hwnd, &rect); // Setup display size (every frame to accommodate for window resizing)
// 				io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
// 
// 				FeedInput();
// 
// 				//ImGui::NewFrame(); // Start the frame
// 			}
		}
	}

	LRESULT Gui::WndProcHandler (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		{
			switch (msg)
			{
				case WM_KEYDOWN:
				case WM_KEYUP:
				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				{
					int down = !((lparam >> 31) & 1);
					int ctrl = GetKeyState(VK_CONTROL) & (1 << 15);

					switch (wparam)
					{
						case VK_SHIFT:
						case VK_LSHIFT:
						case VK_RSHIFT:
							nk_input_key(&m_gfx->m_context, NK_KEY_SHIFT, down);
							return 1;

						case VK_DELETE:
							nk_input_key(&m_gfx->m_context, NK_KEY_DEL, down);
							return 1;

						case VK_RETURN:
							nk_input_key(&m_gfx->m_context, NK_KEY_ENTER, down);
							return 1;

						case VK_TAB:
							nk_input_key(&m_gfx->m_context, NK_KEY_TAB, down);
							return 1;

						case VK_LEFT:
							if (ctrl)
								nk_input_key(&m_gfx->m_context, NK_KEY_TEXT_WORD_LEFT, down);
							else
								nk_input_key(&m_gfx->m_context, NK_KEY_LEFT, down);
							return 1;

						case VK_RIGHT:
							if (ctrl)
								nk_input_key(&m_gfx->m_context, NK_KEY_TEXT_WORD_RIGHT, down);
							else
								nk_input_key(&m_gfx->m_context, NK_KEY_RIGHT, down);
							return 1;

						case VK_BACK:
							nk_input_key(&m_gfx->m_context, NK_KEY_BACKSPACE, down);
							return 1;

						case VK_HOME:
							nk_input_key(&m_gfx->m_context, NK_KEY_TEXT_START, down);
							nk_input_key(&m_gfx->m_context, NK_KEY_SCROLL_START, down);
							return 1;

						case VK_END:
							nk_input_key(&m_gfx->m_context, NK_KEY_TEXT_END, down);
							nk_input_key(&m_gfx->m_context, NK_KEY_SCROLL_END, down);
							return 1;

						case VK_NEXT:
							nk_input_key(&m_gfx->m_context, NK_KEY_SCROLL_DOWN, down);
							return 1;

						case VK_PRIOR:
							nk_input_key(&m_gfx->m_context, NK_KEY_SCROLL_UP, down);
							return 1;

						case 'C':
							if (ctrl) {
								nk_input_key(&m_gfx->m_context, NK_KEY_COPY, down);
								return 1;
							}
							break;

						case 'V':
							if (ctrl) {
								nk_input_key(&m_gfx->m_context, NK_KEY_PASTE, down);
								return 1;
							}
							break;

						case 'X':
							if (ctrl) {
								nk_input_key(&m_gfx->m_context, NK_KEY_CUT, down);
								return 1;
							}
							break;

						case 'Z':
							if (ctrl) {
								nk_input_key(&m_gfx->m_context, NK_KEY_TEXT_UNDO, down);
								return 1;
							}
							break;

						case 'R':
							if (ctrl) {
								nk_input_key(&m_gfx->m_context, NK_KEY_TEXT_REDO, down);
								return 1;
							}
							break;
					}
					return 0;
				}

				case WM_CHAR:
					if (wparam >= 32)
					{
						nk_input_unicode(&m_gfx->m_context, (nk_rune)wparam);
						return 1;
					}
					break;

				case WM_LBUTTONDOWN:
					if (wparam & MK_CONTROL)
					{
						::UpdateWindow(hwnd);
						::SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
						return 0;
					}
					nk_input_button(&m_gfx->m_context, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
					SetCapture(hwnd);
					return 1;

				case WM_LBUTTONUP:
					nk_input_button(&m_gfx->m_context, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
					nk_input_button(&m_gfx->m_context, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
					ReleaseCapture();
					return 1;

				case WM_RBUTTONDOWN:
					nk_input_button(&m_gfx->m_context, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
					SetCapture(hwnd);
					return 1;

				case WM_RBUTTONUP:
					nk_input_button(&m_gfx->m_context, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
					ReleaseCapture();
					return 1;

				case WM_MBUTTONDOWN:
					if (wparam & MK_CONTROL)
					{
						::UpdateWindow(hwnd);
						::SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE | HTCAPTION, 0);
						return 0;
					}
					nk_input_button(&m_gfx->m_context, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
					SetCapture(hwnd);
					return 1;

				case WM_MBUTTONUP:
					nk_input_button(&m_gfx->m_context, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
					ReleaseCapture();
					return 1;

				case WM_MOUSEWHEEL:
					nk_input_scroll(&m_gfx->m_context, nk_vec2(0, (float)(short)HIWORD(wparam) / WHEEL_DELTA));
					return 1;

				case WM_MOUSEMOVE:
					nk_input_motion(&m_gfx->m_context, (short)LOWORD(lparam), (short)HIWORD(lparam));
					return 1;

				case WM_LBUTTONDBLCLK:
					nk_input_button(&m_gfx->m_context, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
					return 1;
			}
		}

		switch (msg)
		{
			case WM_SIZE:
			{
				if (m_gfx->m_dx11 && wparam != SIZE_MINIMIZED)
				{
					unsigned const x = (UINT)LOWORD(lparam);
					unsigned const y = (UINT)HIWORD(lparam);
					TRACE_MSG(LL_DEBUG, CTX_GFX, "WM_SIZE for hwnd=0x%x gui=0x%x dim=(%u,%u)", hwnd, this, (UINT)LOWORD(lparam), (UINT)HIWORD(lparam));
					OnResize(x, y);
				}
				return 0;
			}
			case WM_SYSCOMMAND:
			{
				if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
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
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	NK_API void nk_d3d11_font_stash_begin (struct nk_font_atlas & atlas)
	{
		nk_font_atlas_init_default(&atlas);
		nk_font_atlas_begin(&atlas);
	}

	NK_API void nk_d3d11_font_stash_end (Gfx * m_gfx)
	{
		int w, h;
		const void * image = nk_font_atlas_bake(&m_gfx->m_atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

		/* upload font to texture and create texture view */
		ID3D11Texture2D * font_texture = nullptr;
		HRESULT hr;

		D3D11_TEXTURE2D_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Width = (UINT)w;
		desc.Height = (UINT)h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		{
			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = image;
			data.SysMemPitch = (UINT)(w * 4);
			data.SysMemSlicePitch = 0;
			hr = m_gfx->m_dx11->m_pd3dDevice->CreateTexture2D(&desc, &data, &font_texture);
			//assert(SUCCEEDED(hr));
		}

		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srv;
			memset(&srv, 0, sizeof(srv));
			srv.Format = desc.Format;
			srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv.Texture2D.MipLevels = 1;
			srv.Texture2D.MostDetailedMip = 0;
			hr = m_gfx->m_dx11->m_pd3dDevice->CreateShaderResourceView((ID3D11Resource *)font_texture, &srv, &m_gfx->m_pFontTextureView);
			//assert(SUCCEEDED(hr));
		}
		font_texture->Release();

		nk_font_atlas_end(&m_gfx->m_atlas, nk_handle_ptr(m_gfx->m_pFontTextureView), &m_gfx->m_nullTexture);
		if (m_gfx->m_atlas.default_font)
			nk_style_set_font(&m_gfx->m_context, &m_gfx->m_atlas.default_font->handle);
	}


	bool Gui::Init (bb::GfxWindow * w)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT , "Initializing GUI for hwnd=0x%x", w->m_hwnd);
		GfxWindow * imgui_w = static_cast<GfxWindow *>(w);
		m_hwnd = w->m_hwnd;
		nk_init_default(&m_gfx->m_context, 0);
		nk_buffer_init_default(&m_gfx->m_cmd);

		/* Load Fonts: if none of these are loaded a default font will be used  */
		/* Load Cursor: if you uncomment cursor loading please hide the cursor */
		{
			nk_d3d11_font_stash_begin(m_gfx->m_atlas);
			/*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../extra_font/DroidSans.ttf", 14, 0);*/
			/*struct nk_font *robot = nk_font_atlas_add_from_file(atlas, "../../extra_font/Roboto-Regular.ttf", 14, 0);*/
			/*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
			/*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../extra_font/ProggyClean.ttf", 12, 0);*/
			/*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../extra_font/ProggyTiny.ttf", 10, 0);*/
			/*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../extra_font/Cousine-Regular.ttf", 13, 0);*/
			nk_d3d11_font_stash_end(m_gfx);
			/*nk_style_load_all_cursors(ctx, atlas->cursors);*/
			/*nk_style_set_font(ctx, &droid->handle)*/;
		}

		//set_style(ctx, THEME_BLUE);

		return true;
	}

	bool Gui::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Terminating GUI");
		m_widgets.clear();
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "destroying ImGui context");
		nk_font_atlas_clear(&m_gfx->m_atlas);
		nk_buffer_free(&m_gfx->m_cmd);
		nk_free(&m_gfx->m_context);

		m_gfx->m_context = nk_context { };
		m_gfxWindow = nullptr;
		m_gfx = nullptr;
		return true;
	}

}}


