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
		m_order.reserve(64);
	}

	void RecoverWindowsWidget::UpdateData ()
	{
		m_tasks.clear();
		m_order.clear();
		m_exenames.clear();

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
				m_order.push_back(i);
				wchar_t appnamew[512] = { 0 };
				getAppByWindow(ti.m_hwnd, appnamew, 512);
				m_exenames.push_back(bbstring(appnamew));
			}
		}

		struct sort_fn
		{
		private:
			std::vector<bbstring> & m_src;
		public:
			sort_fn (std::vector<bbstring> & src) : m_src(src) {}
			bool operator() (int i, int j) const { return m_src[i] < m_src[j]; }
		};

		std::sort(m_order.begin(), m_order.end(), sort_fn(m_exenames));
	}


	void RecoverWindowsWidget::DrawUI ()
	{
		if (ImGui::Button("Update Window List"))
			UpdateData();

		Tasks & tasks = BlackBox::Instance().GetTasks();

    for (size_t i = 0, ie = m_order.size(); i < ie; ++i)
    {
			TaskInfo & ti = m_tasks[m_order[i]];
			if (ti.m_config && ti.m_config->m_bbtasks)
        continue;

			if (i > 0 && m_exenames[m_order[i]] != m_exenames[m_order[i - 1]])
				ImGui::Separator();

			bool const is_visible = ::IsWindowVisible(ti.m_hwnd);
			bool chk = is_visible;
			ImGui::PushID(i);
			char appname[768];
			codecvt_utf16_utf8(m_exenames[m_order[i]], appname, 768);
			if (ImGui::Checkbox(appname, &chk))
			{
				if (chk)
					showWindow(ti.m_hwnd, true);
				else
					showWindow(ti.m_hwnd, false);
			}
			ImGui::PopID();

      char name[TaskInfo::e_captionLenMax];
      codecvt_utf16_utf8(ti.m_caption, name, TaskInfo::e_captionLenMax);
			ImGui::Text(name);
    }
	}

}

