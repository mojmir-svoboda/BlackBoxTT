#include "StyleEditorWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void StyleEditorWidget::DrawUI ()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowSize(display, ImGuiSetCond_Always);

		char name[256];
		codecvt_utf16_utf8(GetNameW(), name, 256);
		ImGui::Begin(name, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::ShowStyleEditor();

		ImGui::End();
	}
}
