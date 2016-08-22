#pragma once
#include "TaskConfig.h"
#include <memory>
namespace YAML { class Node; }
namespace bb {

	struct TasksConfig
	{
		std::vector<std::unique_ptr<TaskConfig>> m_tasks;

		void clear () { m_tasks.clear(); }
	};

	bool loadTasksConfig (YAML::Node & y_root, TasksConfig & config);
}

