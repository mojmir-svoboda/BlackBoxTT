#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <imgui/imgui.h>
#include <blackbox/menu/MenuConfig.h>

namespace bb {
namespace imgui {

	struct MenuWidget : GuiWidget
	{
		constexpr static wchar_t const * const c_type = L"Menu";
		MenuConfig m_config;
		size_t m_currentIndex { 0 };
		ImVec2 m_contentSize { 0, 0 };
		MenuWidget ();
		virtual ~MenuWidget ();

		virtual wchar_t const * GetWidgetTypeName () override { return c_type; }
		virtual bool loadConfig (YAML::Node & y_cfg_node) override;
		virtual void DrawUI () override;
		virtual void Show (bool on) override { m_config.m_show = on; }
		virtual bool Visible () const override { return m_config.m_show; }
		virtual bbstring const & GetId () const override { return m_config.m_id; }
		void CreateMenuFromConfig (MenuConfig const & cfg);
	};

	/*struct FolderMenuWidget : MenuWidget
	{
	};

	struct ControlPanelMenuWidget : FolderMenuWidget
	{
		virtual char const * GetName () { return "CtrlPanel"; }
	};*/
}}
