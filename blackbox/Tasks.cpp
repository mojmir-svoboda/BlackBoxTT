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
#include "VirtualDesktopManager.h"

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
	m_tasks.reserve(128);
	return true;
}

bool Tasks::Done ()
{
	TRACE_MSG(LL_INFO, CTX_BB, "Terminating tasks");
	m_tasks.clear();
	return true;
}

void Tasks::MkDataCopy (std::vector<TaskInfo> & p)
{
	m_lock.Lock();

	for (TaskInfoPtr & t : m_tasks)
		if (t)
			p.emplace_back(*t.get());

	m_lock.Unlock();
}

bool Tasks::FindTask (HWND hwnd, size_t & idx)
{
	return static_cast<Tasks const &>(*this).FindTask(hwnd, idx);
}
bool Tasks::FindTask (HWND hwnd, size_t & idx) const
{
	for (size_t i = 0; i < m_tasks.size(); ++i)
		if (m_tasks[i] && m_tasks[i]->m_hwnd == hwnd)
		{
			idx = i;
			return true;
		}
	idx = c_invalidIndex;
	return false;
}

bool Tasks::RmTask (HWND hwnd)
{
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];
		if (ti_ptr)
		{
			TRACE_MSG(LL_DEBUG, CTX_BB, "--- %ws", ti_ptr->m_caption);
			if (ti_ptr.get() == m_active)
			{
				m_active = nullptr;
			}
			m_tasks.erase(m_tasks.begin() + idx);
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
		if (size_t const n = getAppByWindow(ti->m_hwnd, ti->m_appName, TaskInfo::e_appNameLenMax))
		{
			bbstring name(ti->m_appName);

			IconId id;
			if (BlackBox::Instance().FindIconInCache(name, id))
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
						BlackBox::Instance().AddIconToCache(name, sml, sml_id);
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
	size_t idx = c_invalidIndex;
	if (FindTask(w->m_hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];
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
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];
		UpdateTaskInfoCaption(ti_ptr.get());

		return false;
	}
	else
	{
		TaskInfoPtr ti_ptr(new TaskInfo(hwnd));
		AddTaskInfo(ti_ptr.get());
		bbstring const & cap = ti_ptr->m_caption;

		TaskConfig * c = FindTaskConfig(cap);

		bool pinned = false;
		if (m_wspaces.m_vdm->SupportsPinnedViews())
			pinned = m_wspaces.m_vdm->IsPinned(hwnd);

		if (pinned && !c)
		{
			ti_ptr->m_config = MakeTaskConfig(ti_ptr->m_hwnd);
			c = ti_ptr->m_config;
			c->m_sticky = pinned;
		}

		bool is_sticky = pinned;
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
		TRACE_MSG(LL_DEBUG, CTX_BB, "+++ hwnd=0x%x %ws e=%i i=%i", ti_ptr->m_hwnd, cap.c_str(), (ti_ptr->m_config ? ti_ptr->m_config->m_bbtasks : '0'), (ti_ptr->m_config ? ti_ptr->m_config->m_taskman : '0'));

		if (is_current_ws)
		{
			m_tasks.push_back(std::move(ti_ptr));
		}
		else
		{
			m_tasks.push_back(std::move(ti_ptr));
			if (!m_wspaces.IsVertexVDM(vertex_id))
				::ShowWindow(hwnd, SW_HIDE);
		}
		return true;
	}
}

void Tasks::Update ()
{
	m_lock.Lock();

	// @NOTE: this update probably costs some performance, but UWP apps do
	// not trigger hook events when window created
	// (and also triggers destroy hook too late)
	m_taskEnumStorage.clear();
	EnumWindows(taskEnumProc, (LPARAM)&m_taskEnumStorage);

	for (size_t n = 0; n < m_taskEnumStorage.size(); ++n)
		AddTask(m_taskEnumStorage[n]);

	for (size_t i = 0, ie = m_tasks.size(); i < ie; ++i)
	{
		TaskInfoPtr & ti_ptr = m_tasks[i];
		if (ti_ptr)
		{
			if (ti_ptr->IsTaskManIgnored())
			{
				if (!::IsWindow(ti_ptr->m_hwnd))
					m_tasks[i].reset();
				continue;
			}
			else
			{
				bool found = false;
				for (size_t n = 0; n < m_taskEnumStorage.size(); ++n)
					if (m_tasks[i]->m_hwnd == m_taskEnumStorage[n])
						found = true;
				if (!found)
					m_tasks[i].reset();
			}
		}
	}

	// clear empty unique_ptrs
	m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
			[] (TaskInfoPtr const & ti_ptr) { return ti_ptr.get() == nullptr; }), m_tasks.end());

	for (size_t i = 0, ie = m_tasks.size(); i < ie; ++i)
		if (m_tasks[i])
			UpdateTaskInfoCaption(m_tasks[i].get());

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
		TRACE_MSG(LL_DEBUG, CTX_BB, " taskhook create wnd=0x%08x", hwnd);
		m_lock.Lock();
		AddTask(hwnd);
		m_lock.Unlock();
	}
}

void Tasks::OnHookWindowDestroyed (HWND hwnd)
{
	bool const is_app_win = bb::isAppWindow(hwnd);
	if (is_app_win)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB, " taskhook destroy wnd=0x%08x", hwnd);
		m_lock.Lock();
		bool const removed0 = RmTask(hwnd);
		m_lock.Unlock();
	}
}

void Tasks::OnHookWindowActivated (HWND hwnd)
{
	m_lock.Lock();
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
	{
		m_active = m_tasks[idx].get();
		TRACE_MSG(LL_DEBUG, CTX_BB, " *  %ws", m_active->m_caption);
	}
	m_lock.Unlock();
}

LRESULT Tasks::UpdateFromTaskHook (WPARAM wParam, LPARAM lParam)
{
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
		//case HCBT_ACTIVATE:
		case HSHELL_WINDOWACTIVATED:
		{
			HWND const hwnd = reinterpret_cast<HWND>(lParam);
			TRACE_MSG(LL_VERBOSE, CTX_BB, " taskhook activate wnd=0x%08x", hwnd);
			OnHookWindowActivated(hwnd);
			break;
		}
// 		case HCBT_SETFOCUS:
// 		{
// 			HWND const hwnd = reinterpret_cast<HWND>(lParam);
// 			TRACE_MSG(LL_VERBOSE, CTX_BB, " taskhook focus wnd=0x%08x", hwnd);
// 			break;
// 		}
// 		case HCBT_MINMAX:
// 		{
// 			break;
// 		}
// 		case HCBT_MOVESIZE:
// 		{
// 			break;
// 		}
	}
	return 0;
}

void Tasks::SwitchWorkSpace (bbstring const & src_vertex_id, bbstring const & dst_vertex_id, bool notification)
{
	bool const src_is_vdm = m_wspaces.IsVertexVDM(src_vertex_id);
	bool const dst_is_vdm = m_wspaces.IsVertexVDM(dst_vertex_id);

	if (src_vertex_id == dst_vertex_id)
		return;
	TRACE_MSG(LL_VERBOSE, CTX_BB | CTX_WSPACE, "SwitchWorkSpace %ws --> %ws", src_vertex_id.c_str(), dst_vertex_id.c_str());

	m_lock.Lock();

	for (TaskInfoPtr & t : m_tasks)
	{
		if (t)
		{
			if (t->IsSticky())
				continue;

			bool const on_future_ws = t->m_wspace == dst_vertex_id;

			if (t->IsTaskManIgnored())
			{
				int const flag = on_future_ws ? SW_SHOW : SW_HIDE;
				::ShowWindow(t->m_hwnd, flag); // @NOTE: taskman ignored windows are not handled by VDM when switching
			}

			if (!on_future_ws)
			{
				if (!dst_is_vdm)
					::ShowWindow(t->m_hwnd, SW_HIDE);
				continue;
			}
		}
	}

	m_lock.Unlock();

	if (dst_is_vdm && !notification)
		m_wspaces.SwitchDesktop(dst_vertex_id);
}

void Tasks::SetTaskManIgnoredImpl (size_t idx)
{
	TaskInfoPtr & ti_ptr = m_tasks[idx];

	showInFromTaskBar(ti_ptr->m_hwnd, false);

	if (nullptr == ti_ptr->m_config)
		ti_ptr->m_config = MakeTaskConfig(ti_ptr->m_hwnd);

	ti_ptr->m_config->m_taskman = false;

	TRACE_MSG(LL_DEBUG, CTX_BB, "make task ignored hwnd=%x", ti_ptr->m_hwnd);
}

void Tasks::SetTaskManIgnored (HWND hwnd)
{
	m_lock.Lock();

	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
		SetTaskManIgnoredImpl(idx);

	m_lock.Unlock();
}

void Tasks::UnsetTaskManIgnoredImpl (size_t idx)
{
	TaskInfoPtr & ti_ptr = m_tasks[idx];
	TRACE_MSG(LL_DEBUG, CTX_BB, "unset task ignored hwnd=%x", ti_ptr->m_hwnd);
	showInFromTaskBar(ti_ptr->m_hwnd, true);
	if (ti_ptr->m_config)
		ti_ptr->m_config->m_taskman = true;
}
void Tasks::UnsetTaskManIgnored (HWND hwnd)
{
	m_lock.Lock();

	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
		UnsetTaskManIgnoredImpl(idx);
	
	m_lock.Unlock();
}

void Tasks::ToggleTaskManIgnored (HWND hwnd)
{
	m_lock.Lock();

	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];
		if (!ti_ptr->IsTaskManIgnored())
			SetTaskManIgnoredImpl(idx);
		else
			UnsetTaskManIgnoredImpl(idx);
	}
	
	m_lock.Unlock();
}

void Tasks::SetSticky (HWND hwnd)
{
	m_lock.Lock();

	size_t idx = c_invalidIndex;
	if (!FindTask(hwnd, idx))
		AddTask(hwnd);
	if (FindTask(hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];

		if (nullptr == ti_ptr->m_config)
			ti_ptr->m_config = MakeTaskConfig(hwnd);

		TRACE_MSG(LL_DEBUG, CTX_BB, "make task sticky hwnd=%x", ti_ptr->m_hwnd);

		ti_ptr->m_config->m_sticky = true;
		if (m_wspaces.m_vdm->SupportsPinnedViews())
			m_wspaces.m_vdm->SetPinned(hwnd, ti_ptr->m_config->m_sticky);
	}

	m_lock.Unlock();
}

void Tasks::UnsetSticky (HWND hwnd)
{
	m_lock.Lock();

	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];

		if (nullptr == ti_ptr->m_config)
			ti_ptr->m_config = MakeTaskConfig(hwnd);

		ti_ptr->m_config->m_sticky = false;
		if (m_wspaces.m_vdm->SupportsPinnedViews())
			m_wspaces.m_vdm->SetPinned(hwnd, ti_ptr->m_config->m_sticky);
	}
	
	m_lock.Unlock();
}


void Tasks::SetBBTasksIgnored (HWND hwnd)
{
	m_lock.Lock();

// 	size_t idx = c_invalidIndex;
// 	if (FindTask(hwnd, idx))
// 	{
// 		TaskInfoPtr & ti_ptr = m_tasks[idx];
// 
// 		if (nullptr == ti_ptr->m_config)
// 			ti_ptr->m_config = MakeTaskConfig(hwnd);
// 
// 		ti_ptr->m_config->m_bbtasks = false;
// 
// 		TRACE_MSG(LL_DEBUG, CTX_BB, "bbtasks ignores hwnd=%x", ti_ptr->m_hwnd);
// 	}

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
	TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "Move hwnd to vertex: %ws", vertex_id.c_str());

	m_lock.Lock();
	size_t idx = c_invalidIndex;
	if (FindTask(hwnd, idx))
	{
		TaskInfoPtr & ti_ptr = m_tasks[idx];
		if (ti_ptr)
		{
			//bbstring const * current_ws = m_wspaces.GetCurrentVertexId();
			bool const same_ws = vertex_id == ti_ptr->m_wspace;
			bool const is_sticky = ti_ptr->IsSticky();
			bool const is_same_ws = same_ws || is_sticky;

			if (is_same_ws)
			{
			}
			else
			{
				ti_ptr->SetWorkSpace(vertex_id.c_str());

				size_t vdm_idx = 0;
				if (!m_wspaces.IsVertexVDM(vertex_id, vdm_idx))
					::ShowWindow(hwnd, SW_HIDE);
				else
				{
					GUID const & vdm_guid = m_wspaces.GetVertexGUID(vdm_idx);
					m_wspaces.m_vdm->MoveWindowToDesktop(hwnd, vdm_guid);
				}
			}
		}
	}
	m_lock.Unlock();
	return true;
}

bool Tasks::OnSwitchDesktopVDM (bbstring const & src_vertex_id, bbstring const & dst_vertex_id)
{
	TRACE_MSG(LL_DEBUG, CTX_BB | CTX_WSPACE, "VDM desktop changed: %ws --> %ws", src_vertex_id.c_str(), dst_vertex_id.c_str());
	if (bbstring const * curr = m_wspaces.GetCurrentVertexId())
	{
		if (*curr == dst_vertex_id)
			return false; // already there

		m_wspaces.SetCurrentVertexId(dst_vertex_id);
		SwitchWorkSpace(src_vertex_id, dst_vertex_id, true);
		return true;
	}
	return false;
}


}
