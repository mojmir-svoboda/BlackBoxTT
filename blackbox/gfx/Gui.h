#pragma once
#include <vector>
#include <memory>
#include <bblib/bbstring.h>
#include <blackbox/common.h>
#include <minwindef.h>
#include "GuiWidget.h"

namespace bb {
	struct GfxWindow;
	struct GuiWidget;

	struct Gui
	{
		bool m_show { false };
		std::vector<std::unique_ptr<GuiWidget>> m_widgets;
		bbstring m_name { };

		Gui () { }
		virtual ~Gui () { }

		virtual void ResetInput () = 0;
		virtual void NewFrame () = 0;
		virtual void DrawUI () = 0;
		virtual void Render () = 0;
		virtual bool Init (GfxWindow * w) = 0;
		virtual bool Done () = 0;
		virtual GuiWidget * FindWidget (wchar_t const * widgetId) = 0;
		virtual bool RmWidget (GuiWidget * widget) = 0;

		virtual void Show (bool on) { m_show = on; }
		virtual bool Visible () const { return m_show; }

		static LRESULT CALLBACK GuiWndProcDispatch (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		virtual LRESULT WndProcHandler (HWND, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	};
}

