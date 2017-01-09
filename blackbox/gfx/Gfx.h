#pragma once
#include "platform_win.h"
#include <vector>
#include "GfxWindow.h"
#include <WidgetConfig.h>

namespace bb {

	struct Gfx
	{
		using GfxWindowPtr = std::unique_ptr<GfxWindow>;
		bool m_ready { false };
		std::vector<GfxWindowPtr> m_windows;
		std::vector<GfxWindowPtr> m_newWindows;

		Gfx () { }
		virtual ~Gfx ();
		virtual bool Init ();
		virtual void Render () = 0;
		virtual void NewFrame () = 0;
		virtual bool Done () = 0;

		virtual GfxWindow * MkGuiWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show) = 0;
		virtual HWND MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname) = 0;
		virtual GfxWindow * MkGfxWindow (HWND hwnd, Gui * gui, wchar_t const * clname, wchar_t const * wname) = 0;

		bool IsReady () const { return m_ready; }
		GfxWindow * GetGfxWindow (size_t n) { return m_windows[n].get(); }
		GfxWindow const * GetGfxWindow (size_t n) const { return m_windows[n].get(); }

		virtual bool AddIconToCache (bbstring const & name, HICON ico, IconId & id) = 0;
		virtual bool FindIconInCache (bbstring const & name, IconId & id) const = 0;
	};

}

