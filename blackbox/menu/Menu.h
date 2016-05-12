#pragma once
#include <gfx/Gui.h>

namespace bb {

	struct Menu : GuiWidget
	{
		virtual ~Menu ();

		virtual void DrawUI () { }
		virtual char const * GetName () = 0;
	};

	struct FolderMenu : Menu
	{
	};

	struct ControlPanelMenu : FolderMenu
	{
		virtual char const * GetName () { return "CtrlPanel"; }
	};
}
