#include "WorkSpaces.h"
#include "WorkSpacesConfig.h"
#include <bblib/logging.h>
namespace bb {

	WorkSpaces::WorkSpaces ()
	{ }

	WorkSpaces::~WorkSpaces()
	{
	}

	bool WorkSpaces::Init (WorkSpacesConfig & config)
	{
		TRACE_SCOPE(LL_INFO, CTX_BB | CTX_INIT);
		m_config = config;
		return true;
	}

	bool WorkSpaces::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB, "Terminating workspaces");
		return true;
	}

}