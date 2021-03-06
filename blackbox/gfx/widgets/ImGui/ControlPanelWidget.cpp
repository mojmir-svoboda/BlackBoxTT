#include "ControlPanelWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void ControlPanelWidget::DrawUI ()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowSize(display, ImGuiSetCond_Always);

		char name[256];
		codecvt_utf16_utf8(GetNameW(), name, 256);
		ImGui::Begin(name, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		// temporary control panels
		auto const & ctrlp = BlackBox::Instance().GetExplorer().m_controlPanel;
		if (ImGui::TreeNode("CtrlP", "%s", "ctrl"))
		{
			for (ExplorerItem const & it : ctrlp)
			{
				//ExplorerItem const & it = ctrlp[i];
				IconId const icoid = it.m_icoSmall;
				ImGui::Icon(icoid, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				ImGui::SameLine();
				std::string name;
				codecvt_utf16_utf8(it.m_name, name); // @TODO: perf!
				if (ImGui::Button(name.c_str()))
				{
					BlackBox::Instance().GetExplorer().OnClickedAt(it.m_pidl);
				}
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
}
