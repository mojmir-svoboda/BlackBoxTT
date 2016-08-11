#include "RecoverWindowsWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <blackbox/utils_window.h>
#include <blackbox/utils_recover.h>
#include <bblib/codecvt.h>

namespace bb {

	RecoverWindowsWidget::RecoverWindowsWidget ()
	{
		m_tasks.reserve(64);
	}

	void RecoverWindowsWidget::UpdateData ()
	{
		m_tasks.clear();

		ProcessWindows pw[256];
		enumerateProcessHandles(pw, 256);
	}


	void RecoverWindowsWidget::DrawUI ()
	{
		UpdateData();

		Tasks & tasks = BlackBox::Instance().GetTasks();

    for (TaskInfo & t : m_tasks)
    {
      if (t.m_exclude)
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
        tasks.MakeIgnored(t.m_hwnd);
      }
			ImGui::PopID();

      ImGui::SameLine();

      if (ImGui::Button(name))
      {
        tasks.Focus(t.m_hwnd);
      }
    }
	}

}

