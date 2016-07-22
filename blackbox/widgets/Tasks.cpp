#include "Tasks.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void /*Workspaces::*/SwitchToWindow (HWND hwnd_app)
	{
		SwitchToThisWindow(hwnd_app, 1);
// 		HWND hwnd = GetLastActivePopup(GetRootWindow(hwnd_app));
// 		if (have_imp(pSwitchToThisWindow)) {
// 			// this one also restores the window, if it's iconic:
// 			pSwitchToThisWindow(hwnd, 1);
// 		}
// 		else {
// 			SetForegroundWindow(hwnd);
// 			if (IsIconic(hwnd))
// 				send_syscommand(hwnd, SC_RESTORE);
// 		}
	}

	void /*Workspaces::*/WS_BringToFront (HWND hwnd, bool to_current)
	{
		int windesk;

		//CleanTasks();

// 		windesk = vwm_get_desk(hwnd);
// 		if (windesk != currentScreen)
// 		{
// 			if (false == to_current)
// 				switchToDesktop(windesk);
// 			else
// 				setDesktop(hwnd, currentScreen, false);
// 		}
		SwitchToWindow(hwnd);
	}

	bool /*Workspaces::*/FocusTopWindow()
	{
// 		HWND hw = get_top_window(currentScreen);
// 		if (hw) {
// 			SwitchToWindow(hw);
// 			return true;
// 		}
// 		SwitchToBBWnd();
		return false;
	}

	void focus (HWND hwnd)
	{
		if (hwnd)
			//WS_BringToFront(hwnd, 0 != (wParam & BBBTF_CURRENT));
			WS_BringToFront(hwnd, false);
		else
			FocusTopWindow();
	}

	void switchDesk()
	{

	}

	void showInFromTaskBar(HWND hwnd, bool show)
	{
		if (show)
		{
			long style = GetWindowLong(hwnd, GWL_STYLE);
			style &= ~(WS_VISIBLE);    // this works - window become invisible 

			style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
			style &= ~(WS_EX_APPWINDOW);

			ShowWindow(hwnd, SW_HIDE); // hide the window
			SetWindowLong(hwnd, GWL_STYLE, style); // set the style
			ShowWindow(hwnd, SW_SHOW); // show the window for the new style to come into effect
			ShowWindow(hwnd, SW_HIDE); // hide the window so we can't see it
		}
		else
		{
			long style = GetWindowLong(hwnd, GWL_STYLE);
			style &= ~(WS_VISIBLE);    // this works - window become invisible 

			style |= WS_EX_TOOLWINDOW;   // flags don't work - windows remains in taskbar
			style &= ~(WS_EX_APPWINDOW);

			ShowWindow(hwnd, SW_HIDE); // hide the window
			SetWindowLong(hwnd, GWL_STYLE, style); // set the style
			ShowWindow(hwnd, SW_SHOW); // show the window for the new style to come into effect
			ShowWindow(hwnd, SW_HIDE); // hide the window so we can't see it
		}
	}

	void TasksWidget::DrawUI ()
	{

			////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////
			// temporary task list
			Tasks const & tasks = BlackBox::Instance().GetTasks();
			//tasks.m_lock.Lock();
			//if (ImGui::TreeNode("Tasks", "%s", "tasks"))
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
					if (!icoid.IsValid())
					{
						// @TODO: assign color to hwnd?
						ImGui::ColorButton(ImColor(0, 0, 128, 255));
					}
					ImGui::SameLine();

					if (ImGui::Button("i"))
					{
						if (t->m_ignored)
						{
							showInFromTaskBar(t->m_hwnd, true);
							t->m_ignored = false;
						}
						else
						{
							showInFromTaskBar(t->m_hwnd, false);
							t->m_ignored = true;
						}
					}

					ImGui::SameLine();

					if (ImGui::Button(name.c_str()))
					{
						focus(t->m_hwnd);
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
			//tasks.m_lock.Unlock();	}
	}

}

