#pragma once
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/Tasks.h>
#include <imgui/imgui.h>

namespace bb {
namespace imgui {

	struct PagerWidgetConfig : WidgetConfig
	{
	};

	struct PagerWidget : GuiWidget
	{
		constexpr static wchar_t const * const c_type = L"Pager";
		PagerWidgetConfig m_config;
		ImVec2 m_contentSize { 0 , 0 };
		std::vector<TaskInfo> m_tasks;
		HWND m_dragged { nullptr };

		PagerWidget ();
		PagerWidget (PagerWidgetConfig const & cfg) : m_config(cfg) { }
		virtual ~PagerWidget () { }
		virtual void DrawUI () override;
		virtual wchar_t const * GetWidgetTypeName () override { return c_type; }
		virtual bool loadConfig (YAML::Node & y_cfg_node) override;

		virtual void Show (bool on) override { m_config.m_show = on; }
		virtual bool Visible () const override { return m_config.m_show; }
		virtual bbstring const & GetId () const override { return m_config.m_id; }
		virtual PagerWidgetConfig & GetConfig () override { return m_config; }
		virtual PagerWidgetConfig const & GetConfig () const override { return m_config; }

		void UpdateTasks ();
	};

}}
