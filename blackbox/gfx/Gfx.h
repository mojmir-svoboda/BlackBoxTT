#pragma once
#include "platform_win.h"
#include "DX11.h"
#include <vector>
#include "Gui.h"
#include "IconCache.h"
#include "GfxWindow.h"

namespace bb {

	struct Gfx
	{
		DX11 * m_dx11 { nullptr };
		using GfxWindowPtr = std::unique_ptr<GfxWindow>;
		std::vector<GfxWindowPtr> m_windows;
		std::vector<ID3D11Texture2D *> m_textures;
		IconCache m_iconCache;

		Gfx () { }
		~Gfx ();
		bool IsReady () const { return m_dx11 != nullptr; }
		bool Init ();
		void Render ();
		void NewFrame ();
		bool Done ();

		GfxWindow * MkGuiWindow (int x, int y, int w, int h, wchar_t const * clname, wchar_t const * wname);
		HWND MkWindow (void * gui, int x, int y, int w, int h, wchar_t const * clname, wchar_t const * wname);
		GfxWindow * MkGfxWindow (HWND hwnd, Gui * gui, wchar_t const * clname, wchar_t const * wname);
		GfxWindow * GetGfxWindow (size_t n) { return m_windows[n].get(); }
		GfxWindow const * GetGfxWindow (size_t n) const { return m_windows[n].get(); }

		ID3D11ShaderResourceView * MkIconResourceView (uint32_t x, uint32_t y, uint32_t bits, uint8_t * data);
		bool UpdateIconResourceView (uint32_t x, uint32_t y, uint32_t bits, uint8_t * bmpdata, ID3D11ShaderResourceView * view);
		bool AddIconToCache (bbstring const & name, HICON ico, IconId & id)
		{
			return m_iconCache.Add(name, ico, id);
		}
		//void OnResize (unsigned w, unsigned h);
	};

}

