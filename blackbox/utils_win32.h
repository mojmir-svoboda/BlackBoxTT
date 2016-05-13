#pragma once
#include <platform_win.h>
#include <bblib/logging.h>

inline bool mkJobObject (HANDLE & job, bool & in_job)
{
	BOOL isProcessInJob = false;
	if (false == ::IsProcessInJob(GetCurrentProcess(), NULL, &isProcessInJob))
	{
		TRACE_MSG(LL_ERROR, CTX_BB | CTX_HOOK | CTX_INIT, "IsProcessInJob failed");
		return false;
	}
	in_job = isProcessInJob;
	job = ::CreateJobObject(NULL, NULL);
	if (job == nullptr)
	{
		TRACE_MSG(LL_ERROR, CTX_BB | CTX_HOOK | CTX_INIT, "CreateJobObject failed");
		return false;
	}

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (false == ::SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
	{
		TRACE_MSG(LL_ERROR, CTX_BB | CTX_HOOK | CTX_INIT, "SetInformationJobObject failed");
		return false;
	}
	return true;
}


