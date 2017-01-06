#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/menu/MenuConfig.h>

namespace bb {

	struct MenuWidget : GuiWidget
	{
		MenuConfig m_menuConfig;
		size_t m_currentIndex { 0 };
		ImVec2 m_contentSize { 0, 0 };
		MenuWidget (WidgetConfig & cfg);
		virtual ~MenuWidget ();

		virtual void DrawUI () override;
		virtual char const * GetName () override { return "Menu"; }
		virtual wchar_t const * GetNameW () override { return L"Menu"; }
		void CreateMenuFromConfig (MenuConfig const & cfg);
	};

	/*struct FolderMenuWidget : MenuWidget
	{
	};

	struct ControlPanelMenuWidget : FolderMenuWidget
	{
		virtual char const * GetName () { return "CtrlPanel"; }
	};*/
}
