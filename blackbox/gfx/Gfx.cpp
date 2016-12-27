#include "Gfx.h"
#include "DX11.h"
#include "Gui.h"
#include <dxgi1_2.h>
#include <imgui/imgui.h>
#include <utils_window.h>
#include <utils_dwm.h>
#include <bblib/logging.h>

namespace bb {

	Gfx::~Gfx () { }

	bool Gfx::Init ()
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
		return true;
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

	GfxWindow * Gfx::MkGfxWindow (HWND hwnd, Gui * gui, wchar_t const * clname, wchar_t const * wname)
	{
		GfxWindowPtr w(new GfxWindow);
		w->m_hwnd = hwnd;
		w->m_chain = m_dx11->CreateSwapChain(hwnd);
		w->m_view = nullptr; // created in WM_SIZE
		w->m_clName = std::move(bbstring(clname));
		w->m_wName = std::move(bbstring(wname));
		if (!w->m_gui)
		{
			w->m_gui = gui;
			w->m_gui->m_name = wname;
			w->m_gui->m_enabled = true;
			w->m_gui->m_gfxWindow = w.get();
			w->m_gui->m_dx11 = m_dx11;
			w->m_gui->Init(hwnd, m_dx11);
		}
		m_windows.push_back(std::move(w));
		return m_windows.back().get();
	}

	GfxWindow * Gfx::MkGuiWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_GFX);
		Gui * gui = new Gui;
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Created new gui @ 0x%x wname=%ws", gui, wname);
		HWND hwnd = MkWindow(static_cast<void *>(gui), x, y, w, h, alpha, clname, wname);
		GfxWindow * res = MkGfxWindow(hwnd, gui, clname, wname);

		// @TODO: find better place for this
		ImGuiStyle & style = ImGui::GetStyle();
		RECT rect;
		::GetClientRect(hwnd, &rect);
		int const cst = style.WindowRounding; // dumb correction
		createRoundedRect(hwnd, rect.right, rect.bottom, style.WindowRounding, cst);

		::ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
		showInFromTaskBar(hwnd, false);
		return res;
	}

	bool Gfx::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Terminating gfx");
		m_dx11->CleanupDeviceD3D();
		delete m_dx11;
		m_dx11 = nullptr;
		return true;
	}

	void Gfx::NewFrame ()
	{
		for (GfxWindowPtr & w : m_windows)
		{
			w->NewFrame();
		}
	}

	void Gfx::Render ()
	{
		if (IsReady())
		{
			m_iconCache.Update();
			for (GfxWindowPtr & w : m_windows)
			{
				w->Render();
			}
		}
	}

#define USE_DYNAMIC_UPDATE 0

	ID3D11ShaderResourceView * Gfx::MkIconResourceView (uint32_t x, uint32_t y, uint32_t bits, uint8_t * bmpdata)
	{
		ID3D11ShaderResourceView * texview = nullptr;

		D3D11_TEXTURE2D_DESC texdesc = CD3D11_TEXTURE2D_DESC(
				DXGI_FORMAT_B8G8R8A8_UNORM
			,	x, y
			, 1, 1				// arraySize, mipLevels
			, D3D11_BIND_SHADER_RESOURCE		// bindFlags
#if USE_DYNAMIC_UPDATE
			, D3D11_USAGE_DYNAMIC						// dynamic usage. just for testing, default shoud be better (updates of icon textures should be rare)
			, D3D11_CPU_ACCESS_WRITE				// cpuaccessFlags
#else
			, D3D11_USAGE_DEFAULT						// default usage.
			, 0															// cpuaccessFlags
#endif
			, 1, 0, 0			// sampleCount, sampleQuality, miscFlags
			);

		D3D11_SUBRESOURCE_DATA data;
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));

		data.pSysMem = bmpdata;
		data.SysMemPitch = bits / 8 * x; // line size in byte
		data.SysMemSlicePitch = 0;
		ID3D11Texture2D * texture = nullptr;
		HRESULT hr = m_dx11->m_pd3dDevice->CreateTexture2D(&texdesc, &data, &texture);
		if (SUCCEEDED(hr))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
			memset(&viewdesc, 0, sizeof(viewdesc));
			viewdesc.Format = texdesc.Format;
			viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewdesc.Texture2D.MipLevels = 0;
			viewdesc.Texture2D.MostDetailedMip = texdesc.MipLevels;

			HRESULT hr_view = m_dx11->m_pd3dDevice->CreateShaderResourceView(texture, nullptr, &texview);

			texture->Release();
		}
		return texview;
	}

	bool Gfx::UpdateIconResourceView (uint32_t x, uint32_t y, uint32_t bits, uint8_t * bmpdata, ID3D11ShaderResourceView * view)
	{
#if USE_DYNAMIC_UPDATE
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

		m_dx11->m_pd3dDeviceContext->Map(view->Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, bmpdata, x * y * bits / CHAR_BIT);
		m_dx11->m_pd3dDeviceContext->Unmap(vertexBuffer2.Get(), 0);
#else
		D3D11_SUBRESOURCE_DATA data;
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = bmpdata;
		data.SysMemPitch = bits / 8 * x; // line size in byte
		data.SysMemSlicePitch = 0;
		// @NOTE: UpdateSubresource can update only portion of texture.. make use of it @TODO
		//			ID3D11Texture2D * texture = nullptr;
		//			HRESULT hr = m_dx11->m_pd3dDevice->UpdateSubresource();
		//			HRESULT hr = m_dx11->m_pd3dDevice->CreateTexture2D(&texdesc, &data, &texture);
		return true;
#endif
	}

}
