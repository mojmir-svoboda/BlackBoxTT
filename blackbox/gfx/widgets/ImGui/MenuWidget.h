#pragma once
#include <blackbox/gfx/MenuWidget.h>
#include <imgui/imgui.h>

namespace bb {
namespace imgui {

	struct MenuWidget : bb::MenuWidget
	{
		ImVec2 m_contentSize { 0, 0 };
		MenuWidget ();
		MenuWidget (MenuConfig const & cfg) : bb::MenuWidget(cfg) { }
		virtual ~MenuWidget ();

		virtual wchar_t const * GetWidgetTypeName () override { return c_type; }
		virtual bool loadConfig (YAML::Node & y_cfg_node) override;
		virtual void DrawUI () override;
		virtual void Show (bool on) override { m_config.m_show = on; }
		virtual bool Visible () const override { return m_config.m_show; }
		virtual bbstring const & GetId () const override { return m_config.m_id; }
	};
}}
