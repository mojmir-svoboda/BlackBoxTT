#pragma once
#include "platform_win.h"
#include <vector>
#include "GfxWindow.h"
#include "GuiWidget.h"
#include "GfxConfig.h"

namespace bb {

	struct Gfx
	{
		Gfx () { }
		virtual ~Gfx () { }
		virtual bool Init (GfxConfig & cfg) = 0;
		virtual void Render () = 0;
		virtual void NewFrame () = 0;
		virtual bool Done () = 0;

		virtual bool AddIconToCache (bbstring const & name, HICON ico, IconId & id) = 0;
		virtual bool FindIconInCache (bbstring const & name, IconId & id) const = 0;
		virtual bool FindIconCoords (IconId id, void * & texture, float & u0, float & v0, float & u1, float & v1) const = 0;

		virtual GuiWidget * FindWidget (wchar_t const * name) = 0;
		virtual GuiWidget * MkWidgetFromId (wchar_t const * widgetId) = 0;
		//virtual GfxWindow * MkWidgetWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show) = 0;
		//virtual HWND MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname) = 0;
		virtual bool DestroyWindow (wchar_t const * widgetId) = 0;
	};

}

