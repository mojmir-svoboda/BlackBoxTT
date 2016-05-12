#pragma once
#include <windows.h>
#include <tchar.h>
#include <Pdh.h>
#include <vector>
#include <string>
#include <blackbox/worker.h>

struct CoreInfo
{
	int m_core;
	double m_idle;
	double m_kernel;
	double m_proc;
	CoreInfo () : m_core(0), m_idle(0.0), m_kernel(0.0), m_proc(0.0) { }
	CoreInfo (int n, double idl, double ker, double proc) : m_core(n), m_idle(idl), m_kernel(ker), m_proc(proc) { }
};

struct JobWorkerThread;
struct CoreLoad
{
	//@TODO: hide in impl
	HCOUNTER * m_hIdleCounters;
	HCOUNTER * m_hPrivelegedCounters;
	HCOUNTER * m_hProcessorCounters;
	LPTSTR m_szCounterListBuffer;
	LPTSTR m_szInstanceListBuffer;
	HANDLE m_hEvent;
	HQUERY m_hQuery;
	int m_nCount;
	JobWorkerThread * m_worker;
	ThreadPool m_pool;
	std::vector<CoreInfo> m_coreInfos;

	CoreLoad ();
	~CoreLoad ();

	void Update ();
	size_t GetCoreCount () const { return m_nCount; }
	bool HasCoreInfo (size_t i) const { return i < m_coreInfos.size(); }
	CoreInfo const & GetCoreInfo (size_t i) const { return m_coreInfos[i]; }
};

struct JobWorkerThread : WorkerThread
{
	CoreLoad & m_job;
	JobWorkerThread (CoreLoad & job) : m_job(job) { }
	virtual void Run () override
	{
		while (!GetTerminate())
			m_job.Update();
	}
};


