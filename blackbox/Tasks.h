#pragma once
#include <platform_win.h>
#include <vector>
#include <array>
#include <memory>
#include "SpinLock.h"
#include "TaskInfo.h"
#include "TasksConfig.h"
#include "WorkSpaces.h"

namespace bb {

	struct GfxWindow;
	BOOL CALLBACK taskEnumProc (HWND hwnd, LPARAM lParam);
	LRESULT CALLBACK mainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	enum TaskState : unsigned
	{
			e_Active		/// task is in active state (current workspace, not ignored)
		, e_TaskManIgnored		/// task is ignored by user (and task manager)
		, e_OtherWS		/// task is currently on another workspace
		, max_enum_value		/// do not use this as index, please
	};
	using TaskInfoPtr = std::unique_ptr<TaskInfo>;

	class Tasks
	{
		friend BOOL taskEnumProc(HWND hwnd, LPARAM lParam);
		friend LRESULT mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		friend struct BlackBox; friend struct Widgets;

		size_t const c_invalidIndex = std::numeric_limits<size_t>::max();
		SpinLock m_lock;
		TasksConfig * m_config;
		using ptrs_t = std::vector<TaskInfoPtr>;
		using ptrs_it = ptrs_t::iterator;
		using ptrs_cit = ptrs_t::const_iterator;
		std::array<ptrs_t, max_enum_value> m_tasks;
		std::vector<HWND> m_taskEnumStorage;
		TaskInfo * m_active;
		WorkSpaces & m_wspaces;

	public:
		Tasks (WorkSpaces & wspaces);
		~Tasks ();

		bool Init (TasksConfig & config);
		void Update ();
		bool Done ();

		void MkDataCopy (TaskState ts, std::vector<TaskInfo> & p);
		void SwitchWorkSpace (bbstring const & src, bbstring const & dst);
		bool MoveWindowToVertex (HWND hwnd, bbstring const & vertex_id);
		void SetSticky (HWND hwnd);
		void UnsetSticky (HWND hwnd);
		bool IsSticky (HWND hwnd);
		void SetTaskManIgnored (HWND hwnd);
		void UnsetTaskManIgnored (HWND hwnd);
		bool IsTaskManIgnored (HWND hwnd);
		void SetBBTasksIgnored(HWND hwnd);
		void UnsetBBTasksIgnored(HWND hwnd);
		bool IsBBTasksIgnored(HWND hwnd);
		void Focus (HWND hwnd);
		HWND GetActiveTask () const;

	protected:
		bool FindTask (HWND hwnd, TaskState & state, size_t & idx);
		bool FindTask (HWND hwnd, TaskState & state, size_t & idx) const;
		TaskConfig const * FindTaskConfig (bbstring const & cap) const;
		TaskConfig * FindTaskConfig (bbstring const & cap);
		TaskConfig * MakeTaskConfig (HWND hwnd);
		bool RmTask (HWND hwnd);
		void UpdateTaskInfoCaption (TaskInfo * ti);
		void AddTaskInfo (TaskInfo * ti);
		bool AddTask (HWND hwnd);
		bool AddWidgetTask (GfxWindow * w);

		LRESULT UpdateFromTaskHook (WPARAM wParam, LPARAM lParam);
		void OnHookWindowCreated (HWND hwnd);
		void OnHookWindowDestroyed (HWND hwnd);
		void OnHookWindowActivated (HWND hwnd);

	};
}
