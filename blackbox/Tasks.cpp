#pragma once
#include "Tasks.h"
#include "TaskInfo.h"
#include "utils_window.h"
#include "gfx/utils_gdi.h"
#include "logging.h"
#include "BlackBox.h"
#include <cassert>

BOOL CALLBACK taskEnumProc (HWND hwnd, LPARAM lParam);

namespace bb {

Tasks::Tasks ()
	: m_active(nullptr)
{
}

Tasks::~Tasks()
{
	assert(m_tasks.size() == 0);
}

bool Tasks::Init (TasksConfig & config)
{
	TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
	m_config = config;
	m_tasks.reserve(128);
	m_ignored.reserve(32);
	m_otherWS.reserve(128);

	EnumTasks();
	return true;
}

bool Tasks::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating tasks");
	m_tasks.clear();
	m_ignored.clear();
	m_otherWS.clear();
	return true;
}

TaskInfo * Tasks::FindTask (HWND hwnd)
{
	for (TaskInfoPtr & ti : m_tasks)
		if (ti && ti->m_hwnd == hwnd)
			return ti.get();
	return nullptr;
}
TaskInfo const * Tasks::FindTask (HWND hwnd) const
{
	for (TaskInfoPtr const & ti : m_tasks)
		if (ti && ti->m_hwnd == hwnd)
			return ti.get();
	return nullptr;
}

bool Tasks::RmTask (HWND hwnd)
{
	for (size_t i = 0, ie = m_tasks.size(); i < ie; ++i)
	{
		TaskInfoPtr & ti = m_tasks[i];
		if (ti && ti->m_hwnd == hwnd)
		{
			TRACE_MSG(LL_DEBUG, CTX_BB, "--- %ws", ti->m_caption);
			if (ti.get() == m_active)
				m_active = nullptr;
			m_tasks.erase(m_tasks.begin() + i);
			return true;
		}
	}
	return false;
}

void Tasks::UpdateTaskInfo (TaskInfo * ti, bool force)
{
	if (force || wcslen(ti->m_caption) == 0)
		getWindowText(ti->m_hwnd, ti->m_caption, sizeof(ti->m_caption) / sizeof(*ti->m_caption));
	if (force || nullptr == ti->m_icon)
	{
		HICON lrg = getTaskIconLarge(ti->m_hwnd);
		HICON sml = getTaskIconSmall(ti->m_hwnd);
		IconId lrg_id, sml_id;
		BlackBox::Instance().m_gfx.AddIconToCache(ti->m_caption, sml, sml_id);
		BlackBox::Instance().m_gfx.AddIconToCache(ti->m_caption, lrg, lrg_id);
		//get_window_icon(ti->m_val, &tl->icon);
		ti->m_icoSmall = sml_id;
		ti->m_icoLarge = lrg_id;
	}

	//ti->m_active = true;
}


void Tasks::EnumTasks ()
{
	TRACE_SCOPE(LL_VERBOSE, CTX_BB);
	m_lock.Lock();
	EnumWindows(taskEnumProc, 0);
	m_lock.Unlock();
}

bool Tasks::AddTask (HWND hwnd)
{
	if (TaskInfo * ti = FindTask(hwnd))
	{
		UpdateTaskInfo(ti, true);
		return false;
	}
	else
	{
		TaskInfoPtr ti_ptr(new TaskInfo(hwnd));
		UpdateTaskInfo(ti_ptr.get(), true);
		bbstring const & cap = ti_ptr->m_caption;

		bbstring const * current_ws = BlackBox::Instance().GetWorkSpaces().GetCurrentVertexId();

		for (size_t i = 0, ie = m_config.m_tasks.size(); i < ie; ++i)
		{
			TaskConfig & c = m_config.m_tasks[i];
			if (c.MatchCaption(cap))
			{
				ti_ptr->m_config = &c;
				ti_ptr->SetWorkSpace(c.m_wspace.c_str());
				// if ignored
				// if sticky
				break;
			}
		}

		if (current_ws && wcslen(ti_ptr->m_wspace) == 0)
			ti_ptr->SetWorkSpace(current_ws->c_str());
		bool const is_current_ws = *current_ws == ti_ptr->m_wspace;
		TRACE_MSG(LL_DEBUG, CTX_BB, "+++ %ws e=%i i=%i", cap.c_str(), (ti_ptr->m_config ? ti_ptr->m_config->m_exclude : '0'), (ti_ptr->m_config ? ti_ptr->m_config->m_ignore : '0'));

		if (is_current_ws)
		{
			m_tasks.push_back(std::move(ti_ptr));
		}
		else
		{
			m_otherWS.push_back(std::move(ti_ptr));
			::ShowWindow(hwnd, SW_HIDE);
		}
		return true;
	}
}

LRESULT update (WPARAM wParam, LPARAM lParam);

LRESULT Tasks::UpdateFromTaskHook (WPARAM wParam, LPARAM lParam)
{
	TRACE_MSG(LL_DEBUG, CTX_BB, " taskhook wparam=%i", wParam);
	switch (wParam)
	{
		//case HCBT_CREATEWND:
		case HSHELL_WINDOWCREATED:
		{
			m_lock.Lock();
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			AddTask(hwnd);
			m_lock.Unlock();
			break;
		}
		//case HCBT_DESTROYWND:
		case HSHELL_WINDOWDESTROYED:
		{
			m_lock.Lock();
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			bool const removed0 = RmTask(hwnd);
//			bool const removed1 = RmIgnored(hwnd);
//			bool const removed2 = RmOtherWS(hwnd);
			m_lock.Unlock();
			break;
		}
// 		case HSHELL_ACTIVATESHELLWINDOW:
// 			break;
// 		case HSHELL_WINDOWREPLACED:
// 		{
// 			break;
// 		}
		//case HCBT_ACTIVATE:
		case HSHELL_WINDOWACTIVATED:
		{
			m_lock.Lock();
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			if (TaskInfo * ti = FindTask(hwnd))
			{
				m_active = ti;
				TRACE_MSG(LL_DEBUG, CTX_BB, " *  %ws", ti->m_caption);
			}
			m_lock.Unlock();
			break;
		}
		case HSHELL_GETMINRECT:
		{
			break;
		}
		case HSHELL_REDRAW:
		{
			break;
		}
		case HSHELL_TASKMAN:
			//MessageManager_Send(BB_WINKEY, 0, 0);
			break;
// 		case HCBT_MINMAX:
// 		{
// 			break;
// 		}
// 		case HCBT_MOVESIZE:
// 		{
// 			break;
// 		}
// 		case HCBT_SETFOCUS:
// 		{
// 			break;
// 		}
	}
	return 0;
}

void Tasks::HideTasksFromWorkSpace (bbstring const & wspace)
{

}
void Tasks::ShowTasksFromWorkSpace (bbstring const & wspace)
{
}

void Tasks::MakeIgnored (HWND hwnd)
{
	for (TaskInfoPtr & ti : m_tasks)
	{
		if (ti && ti->m_hwnd == hwnd)
		{
			//showInFromTaskBar(ti->m_hwnd, false);

			::ShowWindow(hwnd, SW_HIDE);
			::SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
			::ShowWindow(hwnd, SW_SHOW);

			ti->m_ignore = true;
			TRACE_MSG(LL_DEBUG, CTX_BB, "make task ignored hwnd=%x", ti->m_hwnd);
			m_ignored.push_back(std::move(ti));
		}
	}
}
// void Tasks::MakeIgnored (wchar_t const * caption)
// {
// }

void Tasks::RemoveIgnored (HWND hwnd)
{
	for (TaskInfoPtr & ti : m_ignored)
	{
		if (ti && ti->m_hwnd == hwnd)
		{
			LONG_PTR const flags = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			if (WS_EX_TOOLWINDOW & flags)
			{
				::ShowWindow(hwnd, SW_HIDE);
				::SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
				::ShowWindow(hwnd, SW_SHOW);
			}

			//showInFromTaskBar(ti->m_hwnd, true);
			ti->m_ignore = false;
		}
	}
	
}

void Tasks::Focus (HWND hwnd)
{
	focusWindow(hwnd);
}

void Tasks::Update ()
{
	for (TaskInfoPtr & ti : m_ignored)
	{
		if (ti)
		{
			UpdateTaskInfo(ti.get(), true);
		}
	}
}

}
