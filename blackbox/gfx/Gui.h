#pragma once
#include <vector>
#include <memory>
#include <bblib/bbstring.h>

namespace bb {
	struct GfxWindow;
	struct GuiWidget;

	struct Gui
	{
		bool m_show { false };
		GfxWindow * m_gfxWindow { nullptr };
		std::vector<std::unique_ptr<GuiWidget>> m_widgets;
		bbstring m_name { };

		Gui () { }
		virtual ~Gui () { }

		virtual void NewFrame () = 0;
		virtual void DrawUI () = 0;
		virtual void Render () = 0;
		virtual bool Init (GfxWindow * w) = 0;
		virtual bool Done () = 0;

		virtual void Show (bool on) { m_show = on; }
		virtual bool Visible () const { return m_show; }
	};
}

