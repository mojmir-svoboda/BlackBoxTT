#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/menu/MenuConfig.h>

namespace bb {

	struct MenuWidget : GuiWidget
	{
		constexpr static wchar_t const * const c_type = L"Menu";
		MenuConfig m_config;
		size_t m_currentIndex { 0 };
		MenuWidget () : GuiWidget() { }
		MenuWidget (MenuConfig const & cfg) : m_config(cfg) { }

		virtual wchar_t const * GetWidgetTypeName () override { return c_type; }
		virtual bool loadConfig (YAML::Node & y_cfg_node) override;
		virtual void Show (bool on) override { m_config.m_show = on; }
		virtual bool Visible () const override { return m_config.m_show; }
		virtual bbstring const & GetId () const override { return m_config.m_id; }
		virtual MenuConfig & GetConfig () override { return m_config; }
		virtual MenuConfig const & GetConfig () const override { return m_config; }
	};
}

