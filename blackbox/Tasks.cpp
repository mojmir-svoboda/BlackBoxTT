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
{ }

Tasks::~Tasks()
{
	assert(m_tasks.size() == 0);
}

bool Tasks::Init (TasksConfig & config)
{
	TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
	m_config = config;
	m_tasks.reserve(256);
	EnumTasks();
	return true;
}

bool Tasks::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating tasks");
	m_tasks.clear();
	return true;
}

TaskInfo * Tasks::FindTask (HWND hwnd)
{
	for (TaskInfoPtr & ti : m_tasks)
		if (ti->m_hwnd == hwnd)
			return ti.get();
	return nullptr;
}
TaskInfo const * Tasks::FindTask (HWND hwnd) const
{
	for (TaskInfoPtr const & ti : m_tasks)
		if (ti->m_hwnd == hwnd)
			return ti.get();
	return nullptr;
}

bool Tasks::RmTask (HWND hwnd)
{
	for (size_t i = 0, ie = m_tasks.size(); i < ie; ++i)
	{
		TaskInfoPtr & ti = m_tasks[i];
		if (ti->m_hwnd == hwnd)
		{
			TRACE_MSG(LL_DEBUG, CTX_BB, "--- %ws", ti->m_caption.c_str());
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
	if (force || ti->m_caption.empty())
		getWindowText(ti->m_hwnd, ti->m_caption);
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

	ti->m_wkspc = 0;
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

		for (size_t i = 0, ie = m_config.m_tasks.size(); i < ie; ++i)
		{
			TaskConfig & c = m_config.m_tasks[i];
			if (c.MatchCaption(cap))
			{
				ti_ptr->m_config = &c;
			}
		}

		TRACE_MSG(LL_DEBUG, CTX_BB, "+++ %ws i=%i", cap.c_str(), (ti_ptr->m_config ? ti_ptr->m_config->m_ignored : '0'));
		m_tasks.push_back(std::move(ti_ptr));
		return true;
	}
}

LRESULT update (WPARAM wParam, LPARAM lParam);

LRESULT Tasks::UpdateFromTaskHook (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case HSHELL_WINDOWCREATED:
		{
			m_lock.Lock();
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			AddTask(hwnd);
			m_lock.Unlock();
			break;
		}
		case HSHELL_WINDOWDESTROYED:
		{
			m_lock.Lock();
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			bool const removed = RmTask(hwnd);
			m_lock.Unlock();
			break;
		}
		case HSHELL_ACTIVATESHELLWINDOW:
			break;
		case HSHELL_WINDOWREPLACED:
		{
			break;
		}
		case HSHELL_WINDOWACTIVATED:
		{
			m_lock.Lock();
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			if (TaskInfo * ti = FindTask(hwnd))
			{
				m_active = ti;
				TRACE_MSG(LL_DEBUG, CTX_BB, " *  %ws", ti->m_caption.c_str());
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
	}
	return 0;
}

}
