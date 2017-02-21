#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <imgui/imgui.h>

namespace bb {
namespace imgui {

	struct StyleEditorWidgetConfig : WidgetConfig { };

	struct StyleEditorWidget : GuiWidget
	{
		constexpr static wchar_t const * const c_type = L"StyleEditor";
		StyleEditorWidgetConfig m_config;
		ImVec2 m_contentSize { 0 , 0 };

		StyleEditorWidget () { }
		StyleEditorWidget (StyleEditorWidgetConfig const & cfg) : m_config(cfg) { }
		virtual ~StyleEditorWidget () { }
		virtual void DrawUI () override;
		virtual wchar_t const * GetWidgetTypeName () override { return c_type; }
		virtual bool loadConfig (YAML::Node & y_cfg_node) override;

		virtual void Show (bool on) override { m_config.m_show = on; }
		virtual bool Visible () const override { return m_config.m_show; }
		virtual bbstring const & GetId () const override { return m_config.m_id; }
		virtual StyleEditorWidgetConfig & GetConfig () override { return m_config; }
		virtual StyleEditorWidgetConfig const & GetConfig () const override { return m_config; }
	};
}}

