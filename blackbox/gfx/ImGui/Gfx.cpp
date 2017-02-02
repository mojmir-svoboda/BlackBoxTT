#include "Gfx.h"
#include "DX11.h"
#include "Gui.h"
#include <dxgi1_2.h>
#include <imgui/imgui.h>
#include <utils_window.h>
#include <utils_dwm.h>
#include <bblib/logging.h>
#include <gfx/widgets/ImGui/StyleEditorWidget.h>
#include <gfx/widgets/ImGui/PluginsWidget.h>
#include <gfx/widgets/ImGui/ControlPanelWidget.h>
#include <gfx/widgets/ImGui/RecoverWindowsWidget.h>
#include <gfx/widgets/ImGui/TasksWidget.h>
#include <gfx/widgets/ImGui/PagerWidget.h>
#include <gfx/widgets/ImGui/DebugWidget.h>
#include <gfx/widgets/ImGui/QuickBarWidget.h>
#include <gfx/widgets/ImGui/TrayWidget.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"

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
				bool const ok = MkWidgetFromId(id.c_str());
				if (!ok)
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

// 	template <typename T>
// 	using config_of = decltype(T::m_config);
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

	bool Gfx::MkWidgetFromId (wchar_t const * widgetId)
	{
		YAML::Node & y_widgets = m_y_root["Widgets"];
		if (y_widgets)
		{
			int const n = y_widgets.size();
			for (int i = 0; i < n; ++i)
			{
				YAML::Node & y_widgets_i = y_widgets[i];
				bb::WidgetConfig tmp = y_widgets[i].as<bb::WidgetConfig>();
				if (tmp.m_id == widgetId)
				{
					std::unique_ptr<GuiWidget> w = MkWidgetFromType(tmp.m_widgetType.c_str());
					if (w && w->loadConfig(y_widgets_i))
					{
						GfxWindow * win = MkWidgetWindow(tmp.m_x, tmp.m_y, tmp.m_w, tmp.m_h, tmp.m_alpha, w->GetWidgetTypeName(), w->GetId().c_str(), tmp.m_show);
						w->m_gfxWindow = win;
						win->m_gui->m_widgets.push_back(std::move(w));
						return true;
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

	GfxWindow * Gfx::MkWidgetWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_GFX);
		std::unique_ptr<Gui> gui(new Gui);
		gui->m_gfx = this;
		std::unique_ptr<GfxWindow> gw(new GfxWindow);
		gui->m_gfxWindow = gw.get();
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Created new gui @ 0x%x wname=%ws", gui, wname);
		HWND hwnd = MkWindow(static_cast<void *>(gui.get()), x, y, w, h, alpha, clname, wname);
		gw->m_hwnd = hwnd;
		gw->m_chain = m_dx11->CreateSwapChain(hwnd);
		gw->m_view = nullptr; // created in WM_SIZE @see Gui::WndProcHandler
		gw->m_clName = std::move(bbstring(clname));
		gw->m_wName = std::move(bbstring(wname));
		if (!gw->m_gui)
		{
			gw->m_gui = std::move(gui);
			gw->m_gui->m_name = wname;
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
		return m_newWindows.back().get();
	}

	bool Gfx::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX, "Terminating gfx");
		if (m_dx11)
		{
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

}}
