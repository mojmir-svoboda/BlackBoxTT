#pragma once
#include "TaskConfig.h"
namespace YAML { class Node; }
namespace bb {

	struct TasksConfig
	{
		std::vector<TaskConfig> m_tasks;
	};

	bool loadTasksConfig (YAML::Node & y_root, TasksConfig & config);
}

