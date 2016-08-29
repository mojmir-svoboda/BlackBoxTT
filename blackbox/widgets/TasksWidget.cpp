#include "TasksWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <blackbox/utils_window.h>
#include <bblib/codecvt.h>

namespace bb {

	TasksWidget::TasksWidget ()
	{
		m_tasks.reserve(64);
	}

	void TasksWidget::UpdateTasks ()
	{
		m_tasks.clear();

		Tasks & tasks = BlackBox::Instance().GetTasks();
		tasks.MkDataCopy(e_Active, m_tasks);
	}


	void TasksWidget::DrawUI ()
	{
		UpdateTasks();
		// useless windows
		// Store hwnd == WinStore.Mobile.exe process
		// Get Started hwnd == WhatsNew.Store.exe

			////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////
		Tasks & tasks = BlackBox::Instance().GetTasks();
//      std::string name2;
//				if (tasks.m_active)
//				{
//					codecvt_utf16_utf8(tasks.m_active->m_caption, name2); // @TODO: perf!
//					ImGui::Button(name2.c_str());
//				}
    ImGui::Separator();
    for (TaskInfo & t : m_tasks)
    {
      if (t.m_config && t.m_config->m_bbtasks)
        continue;

      char name[TaskInfo::e_captionLenMax];
      codecvt_utf16_utf8(t.m_caption, name, TaskInfo::e_captionLenMax);
      IconId const icoid = t.m_icoSmall;
      ImGui::Icon(icoid, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
      if (!icoid.IsValid())
      {
        // @TODO: assign color to hwnd?
        ImGui::ColorButton(ImColor(0, 0, 128, 255));
      }
      ImGui::SameLine();

			ImGui::PushID(t.m_hwnd);
      if (ImGui::Button("i"))
      {
        tasks.SetTaskManIgnored(t.m_hwnd);
      }
			ImGui::SameLine();
			if (ImGui::Button("e"))
			{
				tasks.SetBBTasksIgnored(t.m_hwnd);
			}
			ImGui::PopID();

      ImGui::SameLine();

      if (ImGui::Button(name))
      {
        tasks.Focus(t.m_hwnd);
      }
    }
    //ImGui::TreePop();

		ImGui::Separator();
// 		for (TaskInfo & t : m_ignored)
// 		{
// 			if (t.m_exclude)
// 				continue;
// 
// 			char name[TaskInfo::e_captionLenMax];
// 			codecvt_utf16_utf8(t.m_caption, name, TaskInfo::e_captionLenMax);
// 			IconId const icoid = t.m_icoSmall;
// 			ImGui::Icon(icoid, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
// 			if (!icoid.IsValid())
// 			{
// 				// @TODO: assign color to hwnd?
// 				ImGui::ColorButton(ImColor(0, 0, 128, 255));
// 			}
// 			ImGui::SameLine();
// 
// 			ImGui::PushID(t.m_hwnd);
// 			if (ImGui::Button("u"))
// 			{
// 				tasks.MakeIgnored(t.m_hwnd);
// 			}
// 			ImGui::PopID();
// 
// 			ImGui::SameLine();
// 
// 			if (ImGui::Button(name))
// 			{
// 				tasks.Focus(t.m_hwnd);
// 			}
// 		}
	}

}
