#pragma once
#include "WorkSpacesConfig.h"
#include "WorkSpacesGraph.h"

namespace bb {

  struct WorkSpaces
  {
    WorkSpacesConfig m_config;
		WorkSpacesGraph m_graph;

		WorkSpaces ();
		~WorkSpaces ();

		bool Init (WorkSpacesConfig & config);
		bool CreateGraph ();
		void ClearGraph ();
		bool Done ();
	};
}
