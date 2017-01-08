#pragma once
#include "platform_win.h"
#include "DX11.h"
#include <vector>
#include "Gui.h"
#include "IconCache.h"
#include "GfxWindow.h"
#include <WidgetConfig.h>

namespace bb {
namespace imgui {

	struct Gfx
	{
		DX11 * m_dx11 { nullptr };
		using GfxWindowPtr = std::unique_ptr<GfxWindow>;
		std::vector<GfxWindowPtr> m_windows;
		std::vector<GfxWindowPtr> m_newWindows;
		std::vector<ID3D11Texture2D *> m_textures;
		IconCache m_iconCache;

		Gfx () { }
		~Gfx ();
		bool IsReady () const { return m_dx11 != nullptr; }
		bool Init ();
		void Render ();
		void NewFrame ();
		bool Done ();

		GfxWindow * MkGuiWindow (int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname, bool show);
		HWND MkWindow (void * gui, int x, int y, int w, int h, int alpha, wchar_t const * clname, wchar_t const * wname);
		GfxWindow * MkGfxWindow (HWND hwnd, Gui * gui, wchar_t const * clname, wchar_t const * wname);
		GfxWindow * GetGfxWindow (size_t n) { return m_windows[n].get(); }
		GfxWindow const * GetGfxWindow (size_t n) const { return m_windows[n].get(); }

		bool MkIconResourceView (IconSlab & slab);
		bool UpdateIconResourceView (IconSlab & slab);
		bool AddIconToCache (bbstring const & name, HICON ico, IconId & id)
		{
			return m_iconCache.Add(name, ico, id);
		}
		bool FindIconInCache (bbstring const & name, IconId & id) const
		{
			return m_iconCache.Find(name, id);
		}
		//void OnResize (unsigned w, unsigned h);
	};

}}

