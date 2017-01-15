#pragma once
#include "platform_win.h"
#include <vector>
#include "GfxWindow.h"
#include <blackbox/WidgetsConfig.h>

namespace bb {

	struct Gfx
	{
		bool m_ready { false };

		Gfx () { }
		virtual ~Gfx () { }
		virtual bool Init (WidgetsConfig & cfg) = 0;
		virtual void Render () = 0;
		virtual void NewFrame () = 0;
		virtual bool Done () = 0;

		virtual GfxWindow * MkGuiWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show) = 0;
		virtual HWND MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname) = 0;

		bool IsReady () const { return m_ready; }

		virtual bool AddIconToCache (bbstring const & name, HICON ico, IconId & id) = 0;
		virtual bool FindIconInCache (bbstring const & name, IconId & id) const = 0;
	};

}

