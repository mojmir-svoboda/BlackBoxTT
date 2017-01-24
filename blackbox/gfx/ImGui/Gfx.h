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
#include <blackbox/WidgetsConfig.h>
#include <blackbox/Tasks.h>

namespace bb {
namespace imgui {

	struct Gfx : bb::Gfx
	{
		Tasks & m_tasks;
		WidgetsConfig * m_config;
		DX11 * m_dx11 { nullptr };
		std::vector<ID3D11Texture2D *> m_textures;
		using GfxWindowPtr = std::unique_ptr<GfxWindow>;
		std::vector<GfxWindowPtr> m_windows;
		std::vector<GfxWindowPtr> m_newWindows;
		IconCache m_iconCache;

		Gfx (Tasks & t) : m_tasks(t) { }
		virtual ~Gfx ();
		virtual bool Init (WidgetsConfig & config) override;
		bool CreateWidgets (WidgetsConfig & config);
		bool IsReady () const { return m_dx11 != nullptr; }

		virtual void Render () override;
		virtual void NewFrame () override;
		virtual bool Done () override;

		virtual GfxWindow * MkWidgetWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show) override;
		virtual HWND MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname) override;

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
		bool MkIconResourceView (IconSlab & slab);
		bool UpdateIconResourceView (IconSlab & slab);
		void UpdateIconCache ();

		virtual GuiWidget * FindWidget (bbstring const & name) override;

		virtual GuiWidget * MkWidget (WidgetConfig & cfg) override;
		std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType);
		std::unique_ptr<GuiWidget> MkWidgetFromId (wchar_t const * widgetId);
	};

}}

