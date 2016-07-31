#include "Tasks.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <blackbox/utils_window.h>
#include <bblib/codecvt.h>

namespace bb {

	void TasksWidget::DrawUI ()
	{
		// useless windows
		// Store hwnd == WinStore.Mobile.exe process
		// Get Started hwnd == WhatsNew.Store.exe

			////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////
			// temporary task list
			Tasks & tasks = BlackBox::Instance().GetTasks();
			tasks.m_lock.Lock();
			//if (ImGui::TreeNode("Tasks", "%s", "tasks"))
			{
				std::string name2;
				if (tasks.m_active)
				{
					codecvt_utf16_utf8(tasks.m_active->m_caption, name2); // @TODO: perf!
					ImGui::Button(name2.c_str());
				}
				ImGui::Separator();
				for (Tasks::TaskInfoPtr & t : tasks.m_tasks)
				{
					if (t->m_exclude)
						continue;

					std::string name;
					codecvt_utf16_utf8(t->m_caption, name); // @TODO: perf!
					IconId const icoid = t->m_icoSmall;
					ImGui::Icon(icoid, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
					if (!icoid.IsValid())
					{
						// @TODO: assign color to hwnd?
						ImGui::ColorButton(ImColor(0, 0, 128, 255));
					}
					ImGui::SameLine();

					if (ImGui::Button("i"))
					{
						if (t->m_ignore)
						{
							tasks.m_ignored.push_back(std::move(t));
							showInFromTaskBar(t->m_hwnd, true);
							t->m_ignore = false;
						}
						else
						{
							showInFromTaskBar(t->m_hwnd, false);
							t->m_ignore = true;
						}
					}

					ImGui::SameLine();

					if (ImGui::Button(name.c_str()))
					{
						focusWindow(t->m_hwnd);
// 						if (t->m_ignored)
// 						{
// 							showInFromTaskBar(t->m_hwnd, true);
// 							t->m_ignored = false;
// 						}
// 						else
// 						{
// 							showInFromTaskBar(t->m_hwnd, false);
// 							t->m_ignored = true;
// 						}
					}
				}
				//ImGui::TreePop();


				if (ImGui::TreeNode("Ignored", "%s", "ignored"))
				{
					ImGui::TreePop();
				}
			}
			tasks.m_lock.Unlock();
	}

}

