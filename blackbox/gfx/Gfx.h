#pragma once
#include "platform_win.h"
#include <vector>
#include "GfxWindow.h"
#include "GuiWidget.h"
#include "GfxConfig.h"
#include "IconId.h"

namespace bb {

	struct Gfx
	{
		Gfx () { }
		virtual ~Gfx () { }
		virtual bool Init (GfxConfig & cfg) = 0;
		virtual void Render () = 0;
		virtual void ResetInput () = 0;
		virtual void NewFrame () = 0;
		virtual bool Done () = 0;

		virtual bool AddIconToCache (bbstring const & name, HICON ico, IconId & id) = 0;
		virtual bool FindIconInCache (bbstring const & name, IconId & id) const = 0;
		virtual bool FindIconCoords (IconId id, void * & texture, float & u0, float & v0, float & u1, float & v1) const = 0;
		virtual bool FindIconCoords (IconId id, void * & texture, uint32_t & sz_x, uint32_t & sz_y, uint32_t & x0, uint32_t & y0, uint32_t & x1, uint32_t & y1) const = 0;

		virtual GuiWidget * FindWidget (wchar_t const * name) = 0;
		virtual GuiWidget * MkWidgetFromId (wchar_t const * widgetId) = 0;
		virtual bool DestroyWindow (wchar_t const * widgetId) = 0;

		template <class ConfigT>
		GuiWidget * MkWidgetFromConfig (ConfigT const & cfg)
		{
			std::unique_ptr<GuiWidget> w = MkWidgetFromType(cfg.m_widgetType.c_str(), cfg);
			if (w)
			{
				GuiWidget * widget = MkWindowForWidget(cfg.m_x, cfg.m_y, cfg.m_w, cfg.m_h, cfg.m_alpha, std::move(w));
				return widget;
			}
			return nullptr;
		}

		GuiWidget * MkWidget (wchar_t const * widgetType, wchar_t const * id)
		{
			std::unique_ptr<GuiWidget> w = MkWidgetFromType(widgetType);
			if (w)
			{
				WidgetConfig & cfg = w->GetConfig();
				cfg.m_id = std::move(bbstring(id));
				GuiWidget * widget = MkWindowForWidget(cfg.m_x, cfg.m_y, cfg.m_w, cfg.m_h, cfg.m_alpha, std::move(w));
				return widget;
			}
			return nullptr;
		}

	protected:
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType) = 0;
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg) = 0;
		virtual GuiWidget * MkWindowForWidget (int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget) = 0;
	};

}

