#pragma once
#include "platform_win.h"
#include <bblib/bbstring.h>

namespace bb {
	struct Gui;

	struct GfxWindow
	{
		HWND m_hwnd { nullptr };
		Gui * m_gui { nullptr };
		bbstring m_clName { };
		bbstring m_wName { };

		GfxWindow () { }
		virtual ~GfxWindow () { }
		virtual void NewFrame () = 0;
		virtual void DrawUI () = 0;
		virtual void Render () = 0;
		virtual bool Done () = 0;

		virtual void Show (bool on) = 0;
		virtual bool Visible () const = 0;

		Gui * GetGui () { return m_gui; }
		Gui const * GetGui () const { return m_gui; }
		bbstring const & GetName () const { return m_wName; }
	};
}

