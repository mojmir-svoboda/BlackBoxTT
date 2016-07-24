#pragma once
#include "WorkSpacesConfig.h"

namespace bb {

  struct WorkSpaces
  {
    WorkSpacesConfig m_config;

		WorkSpaces ();
		~WorkSpaces ();

		bool Init (WorkSpacesConfig & config);
		bool Done ();
	};
}
