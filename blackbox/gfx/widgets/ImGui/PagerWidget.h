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

		PagerWidget ();
		virtual ~PagerWidget () { }
		virtual void DrawUI () override;
		virtual wchar_t const * GetWidgetTypeName () override { return c_type; }
		virtual bool loadConfig (YAML::Node & y_cfg_node) override;

		void UpdateTasks ();
	};

}}
