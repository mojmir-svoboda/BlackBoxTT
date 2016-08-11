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

		std::unique_ptr<ProcessWindows[]> pw(new ProcessWindows[512]);
		size_t const n = enumerateProcessHandles(pw.get(), 512);

		Tasks & tasks = BlackBox::Instance().GetTasks();
		for (size_t i = 0; i < n; ++i)
		{
			ProcessWindows const & pwi = pw[i];
			if (pwi.m_hwnd)
			{
				TaskInfo ti(pwi.m_hwnd);
				ti.SetCaption(pwi.m_caption);
				m_tasks.emplace_back(ti);
			}
		}
	}


	void RecoverWindowsWidget::DrawUI ()
	{
		if (ImGui::Button("Update Window List"))
			UpdateData();

		Tasks & tasks = BlackBox::Instance().GetTasks();

    for (TaskInfo & t : m_tasks)
    {
      if (t.m_exclude)
        continue;

      char name[TaskInfo::e_captionLenMax];
      codecvt_utf16_utf8(t.m_caption, name, TaskInfo::e_captionLenMax);

			bool const is_visible = ::IsWindowVisible(t.m_hwnd);
			bool chk = is_visible;
			if (ImGui::Checkbox(name, &chk))
			{
				if (chk)
					showWindow(t.m_hwnd, true);
				else
					showWindow(t.m_hwnd, false);
			}
			ImGui::SameLine();
			ImGui::Separator();

			wchar_t appnamew[512] = { 0 };
			
			getAppByWindow(t.m_hwnd, appnamew, 512);
			//HANDLE * h = GetWindowLongPtrW(t.m_hwnd, GWLP_HINSTANCE);
			//GetModuleFileNameEx

			char appname[768];
			codecvt_utf16_utf8(appnamew, appname, 768);
			ImGui::Text(appname);
    }
	}

}

