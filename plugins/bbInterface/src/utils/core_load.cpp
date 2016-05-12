#include "core_load.h"
#include <windows.h>
#include <tchar.h>
#include <Pdh.h>
#include <pdhmsg.h>
#include <cstdio>

#pragma comment(lib, "pdh.lib") // @TODO: to cmake

CoreLoad::CoreLoad ()
	: m_hIdleCounters(nullptr)
	, m_hPrivelegedCounters(nullptr)
	, m_hProcessorCounters(nullptr)
	, m_szCounterListBuffer(nullptr)
	, m_szInstanceListBuffer(nullptr)
	, m_hEvent()
	, m_nCount(0)
	, m_worker(nullptr)
{
	PDH_STATUS pdhStatus = ERROR_SUCCESS;
	DWORD dwCounterListSize = 0;
	DWORD dwInstanceListSize = 0;
	LPTSTR szThisInstance = NULL;

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("MyEvent"));

	// Determine the required buffer size for the data.
	pdhStatus = PdhEnumObjectItems(
		NULL, // real time source
		NULL, // local machine
		TEXT("Processor"), // object to enumerate
		m_szCounterListBuffer, // pass NULL and 0
		&dwCounterListSize, // to get length required
		m_szInstanceListBuffer, // buffer size
		&dwInstanceListSize, //
		PERF_DETAIL_WIZARD, // counter detail level
		0);

	if (pdhStatus == PDH_MORE_DATA)
	{
		// Allocate the buffers and try the call again.
		m_szCounterListBuffer = new TCHAR[dwCounterListSize];
		m_szInstanceListBuffer = new TCHAR[dwInstanceListSize];

		if ((m_szCounterListBuffer != NULL) && (m_szInstanceListBuffer != NULL))
		{
			pdhStatus = PdhEnumObjectItems(
				NULL, // real time source
				NULL, // local machine
				TEXT("Processor"), // object to enumerate
				m_szCounterListBuffer, // buffer to receive counter list
				&dwCounterListSize,
				m_szInstanceListBuffer, // buffer to receive instance list
				&dwInstanceListSize,
				PERF_DETAIL_WIZARD, // counter detail level
				0);

			if (pdhStatus == ERROR_SUCCESS)
			{
				// Walk the instance list. The list can contain one or more null-terminated strings. The last string is followed by a second null-terminator.
				for (szThisInstance = m_szInstanceListBuffer; *szThisInstance != 0; szThisInstance += lstrlen(szThisInstance) + 1)
				{
					if (0 != _tcscmp(szThisInstance, TEXT("_Total")))
					{
						// it's not the toalizer, so count it
						m_nCount++;
					}
				}

				m_coreInfos.reserve(m_nCount);
				m_hIdleCounters = new HCOUNTER[m_nCount];
				m_hPrivelegedCounters = new HCOUNTER[m_nCount];
				m_hProcessorCounters = new HCOUNTER[m_nCount];
				pdhStatus = PdhOpenQuery(NULL, 1, &m_hQuery);

				for (int n = 0; n < m_nCount; n++)
				{
					TCHAR szCounterPath[255];
					_stprintf_s(szCounterPath, 255, TEXT("\\Processor(%d)\\%% Processor Time"), n);
					pdhStatus = PdhAddCounter(m_hQuery, szCounterPath, n, &m_hProcessorCounters[n]);
					if (pdhStatus != ERROR_SUCCESS)
					{
						//_tprintf(TEXT("Couldn't add counter \"%s\": 0x8.8X\n"), szCounterPath, pdhStatus);
						break;
					}

					_stprintf_s(szCounterPath, 255, TEXT("\\Processor(%d)\\%% Idle Time"), n);
					pdhStatus = PdhAddCounter(m_hQuery, szCounterPath, n, &m_hIdleCounters[n]);
					if (pdhStatus != ERROR_SUCCESS)
					{
						//_tprintf(TEXT("Couldn't add counter \"%s\": 0x8.8X\n"), szCounterPath, pdhStatus);
						break;
					}

					_stprintf_s(szCounterPath, 255, TEXT("\\Processor(%d)\\%% Privileged Time"), n);
					pdhStatus = PdhAddCounter(m_hQuery, szCounterPath, n, &m_hPrivelegedCounters[n]);
					if (pdhStatus != ERROR_SUCCESS)
					{
						//_tprintf(TEXT("Couldn't add counter \"%s\": 0x8.8X\n"), szCounterPath, pdhStatus);
						break;
					}
				}
			}
			else
			{
				//_tprintf(TEXT("\nPdhEnumObjectItems failed with %ld."), pdhStatus);
			}
		}
	}
	else
	{
		//_tprintf(TEXT("\nPdhEnumObjectItems failed with %ld."), pdhStatus);
	}
	m_worker = new JobWorkerThread(*this);
	m_pool.Create(*m_worker);
}

CoreLoad::~CoreLoad ()
{
	if (m_worker)
	{
		m_worker->SetTerminate();
		m_pool.WaitForTerminate();
		delete m_worker;
		m_worker = nullptr;
	}
	PdhCloseQuery(m_hQuery);

	delete[] m_hIdleCounters;
	delete[] m_hPrivelegedCounters;
	delete[] m_hProcessorCounters;

	delete[] m_szCounterListBuffer;
	delete[] m_szInstanceListBuffer;
	CloseHandle(m_hEvent);
}

void CoreLoad::Update ()
{
	PDH_STATUS pdhStatus = PdhCollectQueryDataEx(m_hQuery, 1, m_hEvent);

	DWORD dwWaitResult = WaitForSingleObject(m_hEvent, INFINITE);
	if (WAIT_OBJECT_0 == dwWaitResult)
	{
		DWORD dwCounterType = 0;
		// LOCK @TODO @FIXME urgent...ish
		m_coreInfos.clear();
		for (int n = 0; n < m_nCount; n++)
		{
			PDH_FMT_COUNTERVALUE cvIdle, cvPriveleged, cvProcessor;
			pdhStatus = PdhGetFormattedCounterValue(m_hIdleCounters[n], PDH_FMT_DOUBLE, &dwCounterType, &cvIdle);
			if (pdhStatus != ERROR_SUCCESS)
				break;

			pdhStatus = PdhGetFormattedCounterValue(m_hPrivelegedCounters[n], PDH_FMT_DOUBLE, &dwCounterType, &cvPriveleged);
			if (pdhStatus != ERROR_SUCCESS)
				break;

			pdhStatus = PdhGetFormattedCounterValue(m_hProcessorCounters[n], PDH_FMT_DOUBLE, &dwCounterType, &cvProcessor);
			if (pdhStatus != ERROR_SUCCESS)
				break;

			m_coreInfos.push_back(CoreInfo(n, cvIdle.doubleValue, cvPriveleged.doubleValue, cvProcessor.doubleValue));
		}
		// UNLOCK @TODO @FIXME
	}
}


