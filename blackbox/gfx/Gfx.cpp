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

}
