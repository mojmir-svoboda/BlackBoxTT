#include "TrayWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void TrayWidget::DrawUI ()
	{
// 			Tray const & tray = BlackBox::Instance().GetTray();
// 			if (ImGui::TreeNode("Tray", "%s", "tray"))
// 			{
// 				for (TrayItem const & t : tray.m_items)
// 				{
// 					std::string name("tmp");
// 					//codecvt_utf16_utf8(t->m_caption, name); // @TODO: perf!
// 					if (ImGui::Button(name.c_str()))
// 					{
// 						;
// 					}
// 				}
// 				ImGui::TreePop();
// 			}

		/*auto const & ctrlp = BlackBox::Instance().m_explorer->m_Tray;
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
					BlackBox::Instance().m_explorer->OnClickedAt(it.m_pidl);
				}
			}
			ImGui::TreePop();
		}*/
	}
}
