#include "StyleEditorWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <bblib/codecvt.h>
#include <imgui/imgui_internal.h>
#include <blackbox/utils_window.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <bblib/codecvt.h>

namespace YAML {
	template<>
	struct convert<bb::imgui::StyleEditorWidgetConfig>
	{
		static Node encode(bb::imgui::StyleEditorWidgetConfig const & rhs)
		{
			Node node = convert<bb::WidgetConfig>::encode(rhs);
			return node;
		}

		static bool decode (Node const & node, bb::imgui::StyleEditorWidgetConfig & rhs)
		{
			try
			{
				if (convert<bb::WidgetConfig>::decode(node, rhs))
				{
					return true;
				}
			}
			catch (std::exception const & e)
			{
				return false;
			}
			return true;
		}
	};
}

namespace bb {
namespace imgui {

	void StyleEditorWidget::DrawUI ()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowSize(display, ImGuiSetCond_Always);

		char name[256];
		codecvt_utf16_utf8(GetId(), name, 256);
		ImGui::Begin(name, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::ShowStyleEditor();

		ImGui::End();
	}

	bool StyleEditorWidget::loadConfig (YAML::Node & y_cfg_node)
	{
		if (!y_cfg_node.IsNull())
		{
			StyleEditorWidgetConfig tmp = y_cfg_node.as<StyleEditorWidgetConfig>();
			m_config = std::move(tmp);
			return true;
		}
		return false;
	}
}}
