#include "Gui.h"
#include "GfxWindow.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Gfx.h"
#include "DX11.h"
#include "BlackBox.h"
#include "utils_imgui.h"
#include <bblib/codecvt.h>
#include <logging.h>
#include <bblib/ScopeGuard.h>
#include <functional>

namespace bb {
namespace imgui {

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
			ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
			ImGui::SetCurrentContext(m_context);
			scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
			{
				DrawUI();
				ImGui::Render();
				m_gfx->RenderImGui();
			}
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
		ImGuiIO & io = ImGui::GetIO();

		// Read keyboard modifiers inputs
		io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
		//...
	}

	void Gui::NewFrame ()
	{
		if (m_show)
		{
			ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
			ImGui::SetCurrentContext(m_context);
			scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
			{
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
					if (m_gfx->m_dx11 && wParam != SIZE_MINIMIZED)
					{
						unsigned const x = (UINT)LOWORD(lParam);
						unsigned const y = (UINT)HIWORD(lParam);
						TRACE_MSG(LL_DEBUG, CTX_GFX, "WM_SIZE for hwnd=0x%x gui=0x%x dim=(%u,%u)", hwnd, this, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
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


	bool Gui::Init (bb::GfxWindow * w)
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT , "Initializing GUI for hwnd=0x%x", w->m_hwnd);
		GfxWindow * imgui_w = static_cast<GfxWindow *>(w);
		m_hwnd = w->m_hwnd;
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

			ImGuiStyle & style = ImGui::GetStyle();
			style.DisplaySafeAreaPadding = ImVec2(0, 0); // @NOTE: default value (4,4) messes with auto-resizing based on SizeContent

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

	bool Gui::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Terminating GUI");
		m_widgets.clear();
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "destroying ImGui context");
		ImGuiContext * const old_ctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(m_context);
		scope_guard_t on_exit = bb::mkScopeGuard(std::ptr_fun(&ImGui::SetCurrentContext), old_ctx);
		ImGui::DestroyContext(m_context);
		m_context = nullptr;
		m_gfxWindow = nullptr;
		m_gfx = nullptr;
		return true;
	}

}}


