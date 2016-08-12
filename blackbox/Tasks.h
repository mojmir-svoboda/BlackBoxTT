#pragma once
#include <platform_win.h>
#include <vector>
#include <array>
#include <memory>
#include "SpinLock.h"
#include "TaskInfo.h"
#include "TasksConfig.h"

namespace bb {

	BOOL CALLBACK taskEnumProc (HWND hwnd, LPARAM lParam);
	LRESULT CALLBACK mainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	enum TaskState : unsigned
	{
			e_Active		/// task is in active state (current workspace, not ignored)
		, e_Ignored		/// task is ignored by user (and task manager)
		, e_OtherWS		/// task is currently on another workspace
		, max_enum_value		/// do not use this as index, please
	};
	using TaskInfoPtr = std::unique_ptr<TaskInfo>;

	class Tasks
	{
		friend BOOL taskEnumProc(HWND hwnd, LPARAM lParam);
		friend LRESULT mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		size_t const c_invalidIndex = std::numeric_limits<size_t>::max();
		SpinLock m_lock;
		TasksConfig m_config;
		using ptrs_t = std::vector<TaskInfoPtr>;
		using ptrs_it = ptrs_t::iterator;
		using ptrs_cit = ptrs_t::const_iterator;
		std::array<ptrs_t, max_enum_value> m_tasks;
		TaskInfo * m_active;

	public:
		Tasks ();
		~Tasks ();

		bool Init (TasksConfig & config);
		void Update ();
		bool Done ();

		void MkDataCopy (TaskState ts, std::vector<TaskInfo> & p);
		void HideTasksFromWorkSpace (bbstring const & wspace);
		void ShowTasksFromWorkSpace (bbstring const & wspace);
		void MakeSticky (HWND hwnd);
		void RemoveSticky (HWND hwnd);
		bool IsSticky (HWND hwnd);
		void MakeIgnored (HWND hwnd);
		//void MakeIgnored (wchar_t const * caption);
		void RemoveIgnored (HWND hwnd);
		bool IsIgnored (HWND hwnd);
		void Focus (HWND hwnd);

	protected:
		bool FindTask (HWND hwnd, TaskState & state, size_t & idx);
		bool FindTask (HWND hwnd, TaskState & state, size_t & idx) const;
		bool RmTask (HWND hwnd);
		void UpdateTaskInfo (TaskInfo * ti, bool force_update);
		void EnumTasks ();
		bool AddTask (HWND hwnd);

		LRESULT UpdateFromTaskHook (WPARAM wParam, LPARAM lParam);
		void OnHookWindowCreated (HWND hwnd);
		void OnHookWindowDestroyed (HWND hwnd);
		void OnHookWindowActivated (HWND hwnd);

	};
}
