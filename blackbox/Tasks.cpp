#pragma once
#include "Tasks.h"
#include "TaskInfo.h"
#include "utils_window.h"
#include "utils_recover.h"
//#include "utils_uwp.h"
#include "gfx/utils_gdi.h"
#include "logging.h"
#include "BlackBox.h"
#include <cassert>
#include <limits>

namespace bb {

Tasks::Tasks (WorkSpaces & wspaces)
	: m_active(nullptr)
	, m_wspaces(wspaces)
{
	m_taskEnumStorage.reserve(256);
}

Tasks::~Tasks()
{
	assert(m_tasks.size() == 0);
}

bool Tasks::Init (TasksConfig & config)
{
	TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
	m_config = &config;
	m_tasks[TaskState::e_Active].reserve(128);
	m_tasks[TaskState::e_TaskManIgnored].reserve(32);
	m_tasks[TaskState::e_OtherWS].reserve(128);

	Update();
	return true;
}

bool Tasks::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating tasks");
	m_tasks[TaskState::e_Active].clear();
	m_tasks[TaskState::e_TaskManIgnored].clear();
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
		if (ti_ptr)
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
void Tasks::UpdateTaskInfoCaption (TaskInfo * ti)
{
	getWindowText(ti->m_hwnd, ti->m_caption, sizeof(ti->m_caption) / sizeof(*ti->m_caption));
}

void Tasks::AddTaskInfo (TaskInfo * ti)
{
	ti->m_uwp = bb::isUWPWindow(ti->m_hwnd);
	UpdateTaskInfoCaption(ti);

	if (!ti->m_icoSmall.IsValid())
	{
		wchar_t tmp[512];
		if (size_t const n = getAppByWindow(ti->m_hwnd, tmp, 512))
		{
			bbstring name(tmp);

			IconId id;
			if (BlackBox::Instance().m_gfx.FindIconInCache(name, id))
			{
				ti->m_icoSmall = id;
			}
			else
			{
				if (ti->m_uwp)
				{

				}
				else
				{
					if (HICON sml = getTaskIconSmall(ti->m_hwnd))
					{
						IconId sml_id;
						BlackBox::Instance().m_gfx.AddIconToCache(name, sml, sml_id);
						ti->m_icoSmall = sml_id;
					}
				}
			}
		}
	}

	if (!ti->m_icoLarge.IsValid())
	{
//		if (HICON lrg = getTaskIconLarge(ti->m_hwnd))
//		{
//			IconId lrg_id;
//			BlackBox::Instance().m_gfx.AddIconToCache(ti->m_caption, lrg, lrg_id);
//			ti->m_icoLarge = lrg_id;
//		}
	}

	//ti->m_active = true;
}

bool Tasks::AddWidgetTask (GfxWindow * w)
{
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(w->m_hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];
		if (ti_ptr->m_config)
		{
			// already exists
		}
	}
	else
	{
		TaskConfig * cfg = FindTaskConfig(w->m_wName);
		if (!cfg)
		{
			cfg = MakeTaskConfig(w->m_hwnd);
			cfg->m_sticky = true;
		}
	}
	return false;
}

TaskConfig * Tasks::FindTaskConfig (bbstring const & cap)
{
	return const_cast<TaskConfig *>(const_cast<const Tasks *>(this)->FindTaskConfig(cap));
}

TaskConfig const * Tasks::FindTaskConfig (bbstring const & cap) const
{
	for (size_t i = 0, ie = m_config->m_tasks.size(); i < ie; ++i)
	{
		TaskConfig const * c = m_config->m_tasks[i].get();
		if (c->MatchCaption(cap))
		{
			return c;
		}
	}
	return nullptr;
}

TaskConfig * Tasks::MakeTaskConfig (HWND hwnd)
{
	std::unique_ptr<TaskConfig> tc(new TaskConfig);
	
	wchar_t cap[TaskInfo::e_captionLenMax];
	getWindowText(hwnd, cap, TaskInfo::e_captionLenMax);
	tc->m_caption = std::move(bbstring(cap));
	m_wspaces.AssignWorkSpace(hwnd, tc->m_wspace);
	m_config->m_tasks.push_back(std::move(tc));

	return m_config->m_tasks.back().get();
}

bool Tasks::AddTask (HWND hwnd)
{
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];
		UpdateTaskInfoCaption(ti_ptr.get());

//		if (ti_ptr->m_config && ti_ptr->m_config->m_sticky)
//			ti_ptr->SetWorkSpace(current_ws->c_str());

//		TaskState ts_new = e_Active;
//		if (ti_ptr->m_config)
//		{
//			if (!ti_ptr->m_config->m_taskman)
//				ts_new = e_TaskManIgnored;
//			else if (!ti_ptr->m_config->m_bbtasks)
//				ts_new = e_BBIgnored;
//		}
// 
//		m_tasks[ts_new].push_back(std::move(ti_ptr));
//		m_tasks[ts].erase(m_tasks[ts].begin() + idx);
// 
		return false;
	}
	else
	{
		TaskInfoPtr ti_ptr(new TaskInfo(hwnd));
		AddTaskInfo(ti_ptr.get());
		bbstring const & cap = ti_ptr->m_caption;

		TaskConfig * c = FindTaskConfig(cap);
		bool is_sticky = false;
		if (c)
		{
			ti_ptr->m_config = c;
			ti_ptr->SetWorkSpace(c->m_wspace.c_str());
			is_sticky = c->m_sticky;
			// if ignored
			// if sticky
		}

		bbstring vertex_id;
		m_wspaces.AssignWorkSpace(ti_ptr->m_hwnd, vertex_id);
		ti_ptr->SetWorkSpace(vertex_id.c_str());

		bbstring const * current_ws = m_wspaces.GetCurrentVertexId();
		bool const same_ws = *current_ws == ti_ptr->m_wspace;
		bool const is_current_ws =	same_ws || is_sticky;
		TRACE_MSG(LL_DEBUG, CTX_BB, "+++ %ws e=%i i=%i", cap.c_str(), (ti_ptr->m_config ? ti_ptr->m_config->m_bbtasks : '0'), (ti_ptr->m_config ? ti_ptr->m_config->m_taskman : '0'));

		if (is_current_ws)
		{
			TaskState ts_new = e_Active;
			if (ti_ptr->m_config)
			{
				if (!ti_ptr->m_config->m_taskman)
					ts_new = e_TaskManIgnored;
			}

			m_tasks[ts_new].push_back(std::move(ti_ptr));
		}
		else
		{
			m_tasks[e_OtherWS].push_back(std::move(ti_ptr));
			if (!m_wspaces.IsVertexVDM(vertex_id))
				::ShowWindow(hwnd, SW_HIDE);
		}
		return true;
	}
}

void Tasks::Update ()
{
	m_lock.Lock();

	// clear empty unique_ptrs
	for (size_t s = 0; s < m_tasks.size(); ++s)
		m_tasks[s].erase(std::remove_if(m_tasks[s].begin(), m_tasks[s].end(),
			[] (TaskInfoPtr const & ti_ptr) { return ti_ptr.get() == nullptr; }), m_tasks[s].end());

	// @NOTE: this update probably costs some performance, but UWP apps do
	// not trigger hook events when window created
	// (and also triggers destroy hook too late)
	m_taskEnumStorage.clear();
	EnumWindows(taskEnumProc, (LPARAM)&m_taskEnumStorage);

	for (size_t n = 0; n < m_taskEnumStorage.size(); ++n)
	{
		AddTask(m_taskEnumStorage[n]);
	}

	for (size_t s = 0; s < m_tasks.size(); ++s)
	{
		if (s == e_OtherWS)
			continue; // do not delete tasks on other wspaces

		for (size_t i = 0, ie = m_tasks[s].size(); i < ie; ++i)
			if (m_tasks[s][i])
			{
				bool found = false;
				for (size_t n = 0; n < m_taskEnumStorage.size(); ++n)
					if (m_tasks[s][i]->m_hwnd == m_taskEnumStorage[n])
						found = true;
				if (!found)
					m_tasks[s][i].reset();
			}
	}

	// @TOD:
	for (size_t s = 0; s < m_tasks.size(); ++s)
		for (size_t i = 0, ie = m_tasks[s].size(); i < ie; ++i)
			if (m_tasks[s][i])
				UpdateTaskInfoCaption(m_tasks[s][i].get());

	m_lock.Unlock();
}

BOOL CALLBACK taskEnumProc (HWND hwnd, LPARAM lParam)
{
	if (std::vector<HWND> * storage = reinterpret_cast<std::vector<HWND> *>(lParam))
	{
		if (bb::isAppWindow(hwnd))
			storage->push_back(hwnd);
	}
	return TRUE;
}

void Tasks::OnHookWindowCreated (HWND hwnd)
{
	bool const is_app_win = bb::isAppWindow(hwnd);
	if (is_app_win)
	{
		m_lock.Lock();
		AddTask(hwnd);
		m_lock.Unlock();
	}
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
		case HSHELL_ACTIVATESHELLWINDOW:
			break;
		
		case HSHELL_ENDTASK:
			break;
		case HSHELL_WINDOWREPLACING:
			break;
		case HSHELL_WINDOWREPLACED:
		{
			break;
		}
		case HSHELL_MONITORCHANGED:
		{
			break;
		}
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
//		case HCBT_MINMAX:
//		{
//			break;
//		}
//		case HCBT_MOVESIZE:
//		{
//			break;
//		}
//		case HCBT_SETFOCUS:
//		{
//			break;
//		}
	}
	return 0;
}

void Tasks::SwitchWorkSpace (bbstring const & src_vertex_id, bbstring const & dst_vertex_id)
{
	bool const src_is_vdm = m_wspaces.IsVertexVDM(src_vertex_id);
	bool const dst_is_vdm = m_wspaces.IsVertexVDM(dst_vertex_id);

	if (src_vertex_id == dst_vertex_id)
		return;

	m_lock.Lock();

	for (TaskInfoPtr & t : m_tasks[e_Active])
		if (t)
		{
			if (t->m_config && t->m_config->m_sticky)
				continue;

			if (t->m_wspace != dst_vertex_id)
			{
				if (!dst_is_vdm)
					::ShowWindow(t->m_hwnd, SW_HIDE);
				m_tasks[e_OtherWS].push_back(std::move(t));
				continue;
			}
		}

	for (TaskInfoPtr & t : m_tasks[e_TaskManIgnored])
		if (t)
		{
			if (t->m_config && t->m_config->m_sticky)
				continue;

			if (t->m_wspace != dst_vertex_id)
			{
				if (!dst_is_vdm)
					::ShowWindow(t->m_hwnd, SW_HIDE);
				m_tasks[e_OtherWS].push_back(std::move(t));
				continue;
			}
		}

	for (TaskInfoPtr & t : m_tasks[e_OtherWS])
		if (t)
		{
			if (t->m_wspace == dst_vertex_id)
			{
				if (!dst_is_vdm)
					::ShowWindow(t->m_hwnd, SW_SHOW);
				m_tasks[e_Active].push_back(std::move(t));
				continue;
			}
		}

	m_lock.Unlock();

	if (dst_is_vdm)
		m_wspaces.SwitchDesktop(dst_vertex_id);
}

void Tasks::SetTaskManIgnored (HWND hwnd)
{
	m_lock.Lock();

	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];

		showInFromTaskBar(ti_ptr->m_hwnd, false);

		if (nullptr == ti_ptr->m_config)
			ti_ptr->m_config = MakeTaskConfig(hwnd);

		ti_ptr->m_config->m_taskman = false;

		TRACE_MSG(LL_DEBUG, CTX_BB, "make task ignored hwnd=%x", ti_ptr->m_hwnd);
		if (ts != e_TaskManIgnored)
			m_tasks[e_TaskManIgnored].push_back(std::move(ti_ptr));
	}

	m_lock.Unlock();
}

void Tasks::UnsetTaskManIgnored (HWND hwnd)
{
	m_lock.Lock();

	for (TaskInfoPtr & ti_ptr : m_tasks[e_TaskManIgnored])
	{
		if (ti_ptr && ti_ptr->m_hwnd == hwnd)
		{
			showInFromTaskBar(ti_ptr->m_hwnd, true);
			if (ti_ptr->m_config)
				ti_ptr->m_config->m_taskman = true;
		}
	}
	
	m_lock.Unlock();
}

void Tasks::SetSticky (HWND hwnd)
{
	m_lock.Lock();

	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];

		if (nullptr == ti_ptr->m_config)
			ti_ptr->m_config = MakeTaskConfig(hwnd);

		ti_ptr->m_config->m_sticky = true;

		TRACE_MSG(LL_DEBUG, CTX_BB, "make task sticky hwnd=%x", ti_ptr->m_hwnd);
//		if (ts == e_OtherWS)
//			m_tasks[e_Active].push_back(std::move(ti_ptr));
	}

	m_lock.Unlock();
}

void Tasks::UnsetSticky (HWND hwnd)
{
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];

		if (nullptr == ti_ptr->m_config)
			ti_ptr->m_config = MakeTaskConfig(hwnd);

		ti_ptr->m_config->m_sticky = false;
	}
	
	m_lock.Unlock();
}


void Tasks::SetBBTasksIgnored (HWND hwnd)
{
	m_lock.Lock();

	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];

		if (nullptr == ti_ptr->m_config)
			ti_ptr->m_config = MakeTaskConfig(hwnd);

		ti_ptr->m_config->m_bbtasks = false;

		TRACE_MSG(LL_DEBUG, CTX_BB, "bbtasks ignores hwnd=%x", ti_ptr->m_hwnd);
	}

	m_lock.Unlock();
}

void Tasks::UnsetBBTasksIgnored (HWND hwnd)
{
	m_lock.Lock();

//	for (TaskInfoPtr & ti_ptr : m_tasks[e_Active])
//	{
//		if (ti_ptr && ti_ptr->m_hwnd == hwnd)
//		{
//			if (ti_ptr->m_config)
//				ti_ptr->m_config->m_taskman = true;
//		}
//	}
//	
	m_lock.Unlock();
}


void Tasks::Focus (HWND hwnd)
{
	focusWindow(hwnd);
}

HWND Tasks::GetActiveTask () const
{
	HWND hwnd = nullptr;
	m_lock.Lock();
	if (m_active)
	{
		hwnd = m_active->m_hwnd;
	}
	m_lock.Unlock();
	return hwnd;
}

bool Tasks::MoveWindowToVertex (HWND hwnd, bbstring const & vertex_id)
{
	m_lock.Lock();
	TaskState ts = TaskState::max_enum_value;
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, ts, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[ts][idx];
		if (ti_ptr)
		{
			ti_ptr->SetWorkSpace(vertex_id.c_str());

			bbstring const * current_ws = m_wspaces.GetCurrentVertexId();
			bool const same_ws = *current_ws == ti_ptr->m_wspace;
			bool const is_sticky = ti_ptr->m_config ? ti_ptr->m_config->m_sticky : false;
			bool const is_current_ws = same_ws || is_sticky;

			if (is_current_ws)
			{
				TaskState ts_new = e_Active;
				if (ti_ptr->m_config)
				{
					if (!ti_ptr->m_config->m_taskman)
						ts_new = e_TaskManIgnored;
				}

				m_tasks[ts_new].push_back(std::move(ti_ptr));
			}
			else
			{
				m_tasks[e_OtherWS].push_back(std::move(ti_ptr));
				if (!m_wspaces.IsVertexVDM(vertex_id))
					::ShowWindow(hwnd, SW_HIDE);
			}
		}
	}
	m_lock.Unlock();
	return true;
}

}
