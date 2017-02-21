#pragma once
#include <platform_win.h>
#include <vector>
#include <WidgetConfig.h>
#include "GfxWindow.h"
namespace YAML { class Node; }

namespace bb
{
	struct GuiWidget
	{
		GfxWindow * m_gfxWindow { nullptr };

		GuiWidget () { }
		virtual ~GuiWidget () { }
		virtual void DrawUI () = 0;
		virtual wchar_t const * GetWidgetTypeName () = 0;
		virtual WidgetConfig & GetConfig () = 0;
		virtual WidgetConfig const & GetConfig () const = 0;
		virtual bbstring const & GetId () const = 0;
		virtual bool loadConfig (YAML::Node & y_cfg_node) = 0;
		//virtual bool saveConfig (YAML::Node & y_cfg_node) = 0;

		virtual void Show (bool on) = 0;
		virtual bool Visible () const = 0;
		virtual void MoveWindow (int x, int y) { m_gfxWindow->MoveWindow(x, y); }
	};
}

