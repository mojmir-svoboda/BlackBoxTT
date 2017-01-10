#include "QuickBarWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	QuickBarWidget::QuickBarWidget (WidgetConfig & cfg)
		: GuiWidget(cfg)
	{
	}

	void QuickBarWidget::DrawUI ()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowSize(display, ImGuiSetCond_Always);

		char name[256];
		codecvt_utf16_utf8(GetNameW(), name, 256);
		//if (m_config.m_titlebar)
		ImGui::Begin(name, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

		//char idu8[];
		//codecvt_utf16_utf8(vertex_id, idu8, TaskInfo::e_wspaceLenMax);
		//if (ImGui::Button(idu8))
		{ 
		}
		ImGui::End();
	}

}

