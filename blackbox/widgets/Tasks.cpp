#include "Tasks.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void TasksWidget::DrawUI ()
	{

			////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////
			// temporary task list
			Tasks const & tasks = BlackBox::Instance().GetTasks();
			//tasks.m_lock.Lock();
			if (ImGui::TreeNode("Tasks", "%s", "tasks"))
			{
				std::string name2;
				if (tasks.m_active)
				{
					codecvt_utf16_utf8(tasks.m_active->m_caption, name2); // @TODO: perf!
					ImGui::Button(name2.c_str());
				}
				ImGui::Separator();
				for (Tasks::TaskInfoPtr const & t : tasks.m_tasks)
				{
					std::string name;
					codecvt_utf16_utf8(t->m_caption, name); // @TODO: perf!
					IconId const icoid = t->m_icoSmall;
					ImGui::Icon(icoid, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
					if (icoid.IsValid())
						ImGui::SameLine();
					if (ImGui::Button(name.c_str()))
					{
						;
					}
				}
				ImGui::TreePop();
			}
			//tasks.m_lock.Unlock();	}
	}

}

