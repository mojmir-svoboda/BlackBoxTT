#pragma once
#include "Tasks.h"
#include "TaskInfo.h"
#include "utils_window.h"
#include "gfx/utils_gdi.h"
#include "logging.h"
#include "BlackBox.h"
#include <cassert>
#include <limits>

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
	m_tasks[TaskState::e_Active].reserve(128);
	m_tasks[TaskState::e_Ignored].reserve(32);
	m_tasks[TaskState::e_OtherWS].reserve(128);

	EnumTasks();
	return true;
}

bool Tasks::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating tasks");
	m_tasks[TaskState::e_Active].clear();
	m_tasks[TaskState::e_Ignored].clear();
	m_tasks[TaskState::e_OtherWS].clear();
	return true;
}

void Tasks::MkDataCopy (TaskState ts, std::vector<TaskInfo> & p)
{
	m_lock.Lock();

	for (TaskInfoPtr & t : m_tasks[ts])
		if (t)
			p.emplace_back(*t.get());

	m_lock.Unlock();
}

bool Tasks::FindTask (HWND hwnd, TaskState & state, size_t & idx)
{
	return static_cast<Tasks const &>(*this).FindTask(hwnd, state, idx);
}
bool Tasks::FindTask (HWND hwnd, TaskState & state, size_t & idx) const
{
	for (size_t s = 0; s < m_tasks.size(); ++s)
		for (size_t i = 0, ie = m_tasks[s].size(); i < ie; ++i)
			if (m_tasks[s][i] && m_tasks[s][i]->m_hwnd == hwnd)
			{
				state = static_cast<TaskState>(s);
				idx = i;
				return true;
			}
	state = max_enum_value;
	idx = c_invalidIndex;
	return false;
}

bool Tasks::RmTask (HWND hwnd)
{
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];
		if (ti_ptr && ti_ptr->m_hwnd == hwnd)
		{
			TRACE_MSG(LL_DEBUG, CTX_BB, "--- %ws", ti_ptr->m_caption);
			if (ti_ptr.get() == m_active)
			{
				m_active = nullptr;
			}
			if (ts == e_Active)
				m_tasks[ts].erase(m_tasks[ts].begin() + idx);
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

BOOL CALLBACK taskEnumProc(HWND hwnd, LPARAM lParam)
{
	if (bb::isAppWindow(hwnd))
	{
		bb::BlackBox::Instance().GetTasks().AddTask(hwnd);
	}
	return TRUE;
}

void Tasks::EnumTasks ()
{
	TRACE_SCOPE(LL_VERBOSE, CTX_BB);
	m_lock.Lock();
	EnumWindows(taskEnumProc, (LPARAM)this);
	m_lock.Unlock();
}

bool Tasks::AddTask (HWND hwnd)
{
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		UpdateTaskInfo(m_tasks[ts][idx].get(), true);
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
			if (ti_ptr->m_config && ti_ptr->m_config->m_ignore)
				m_tasks[e_Ignored].push_back(std::move(ti_ptr));
			else
				m_tasks[e_Active].push_back(std::move(ti_ptr));
		}
		else
		{
			m_tasks[e_OtherWS].push_back(std::move(ti_ptr));
			::ShowWindow(hwnd, SW_HIDE);
		}
		return true;
	}
}

//LRESULT update (WPARAM wParam, LPARAM lParam);

void Tasks::OnHookWindowCreated (HWND hwnd)
{
	m_lock.Lock();
	AddTask(hwnd);
	m_lock.Unlock();
}

void Tasks::OnHookWindowDestroyed (HWND hwnd)
{
	m_lock.Lock();
	bool const removed0 = RmTask(hwnd);
	m_lock.Unlock();
}

void Tasks::OnHookWindowActivated (HWND hwnd)
{
	m_lock.Lock();
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		m_active = m_tasks[ts][idx].get();
		TRACE_MSG(LL_DEBUG, CTX_BB, " *  %ws", m_active->m_caption);
	}
	m_lock.Unlock();
}

LRESULT Tasks::UpdateFromTaskHook (WPARAM wParam, LPARAM lParam)
{
	TRACE_MSG(LL_DEBUG, CTX_BB, " taskhook wparam=%i", wParam);
	switch (wParam)
	{
		//case HCBT_CREATEWND:
		case HSHELL_WINDOWCREATED:
		{
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			OnHookWindowCreated(hwnd);
			break;
		}
		//case HCBT_DESTROYWND:
		case HSHELL_WINDOWDESTROYED:
		{
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			OnHookWindowDestroyed(hwnd);
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
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			OnHookWindowActivated(hwnd);
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
	m_lock.Lock();
	m_lock.Unlock();
}
void Tasks::ShowTasksFromWorkSpace (bbstring const & wspace)
{
	m_lock.Lock();
	m_lock.Unlock();
}

void Tasks::MakeIgnored (HWND hwnd)
{
	m_lock.Lock();

	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];

		//showInFromTaskBar(ti->m_hwnd, false); --> does not work @TODO @FIXMe
		::ShowWindow(hwnd, SW_HIDE);
		::SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		::ShowWindow(hwnd, SW_SHOW);

		ti_ptr->m_ignore = true;
		TRACE_MSG(LL_DEBUG, CTX_BB, "make task ignored hwnd=%x", ti_ptr->m_hwnd);
		if (ts != e_Ignored)
			m_tasks[e_Ignored].push_back(std::move(ti_ptr));
	}

	m_lock.Unlock();
}
// void Tasks::MakeIgnored (wchar_t const * caption)
// {
// }

void Tasks::RemoveIgnored (HWND hwnd)
{
	m_lock.Lock();

	for (TaskInfoPtr & ti : m_tasks[e_Ignored])
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
	
	m_lock.Unlock();
}

void Tasks::Focus (HWND hwnd)
{
	focusWindow(hwnd);
}

void Tasks::Update ()
{
	m_lock.Lock();

	//ten update je nakej rozbitej
		;
	;
	;
	if (1)
	;
// 	for (size_t s = 0; s < m_tasks.size(); ++s)
// 		for (size_t i = 0, ie = m_tasks[s].size(); i < ie; ++i)
// 			if (m_tasks[s][i])
// 				UpdateTaskInfo(m_tasks[s][i].get(), true);

	m_lock.Unlock();
}

}
