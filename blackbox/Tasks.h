#pragma once
#include <platform_win.h>
#include <vector>
#include <memory>
#include "SpinLock.h"
#include "TaskInfo.h"
#include "TasksConfig.h"

namespace bb {

	struct Tasks
	{
		SpinLock m_lock;
		using TaskInfoPtr = std::unique_ptr<TaskInfo>;
		TasksConfig m_config;
		std::vector<TaskInfoPtr> m_tasks;
		std::vector<TaskInfoPtr> m_ignored;
		TaskInfo * m_active;

		Tasks ();
		~Tasks ();

		bool Init (TasksConfig & config);
		bool Done ();

		TaskInfo * FindTask (HWND hwnd);
		TaskInfo const * FindTask (HWND hwnd) const;
		bool RmTask (HWND hwnd);
		void UpdateTaskInfo (TaskInfo * ti, bool force_update);
		void EnumTasks ();
		bool AddTask (HWND hwnd);

		LRESULT UpdateFromTaskHook (WPARAM wParam, LPARAM lParam);

		void MakeSticky (HWND hwnd);
		void REemoveSticky (HWND hwnd);
		bool IsSticky (HWND hwnd);
		void MakeIgnored (HWND hwnd);
		//void MakeIgnored (wchar_t const * caption);
		void RemoveIgnored (HWND hwnd);
		bool IsIgnored (HWND hwnd);
		void Focus (HWND hwnd);

		void HideTasksFromWorkSpace (bbstring const & wspace);
		void ShowTasksFromWorkSpace (bbstring const & wspace);
	};
}
