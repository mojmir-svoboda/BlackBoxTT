#pragma once
#include <platform_win.h>
#include <SpinLock.h>
#include <shellapi.h>
#include <array>

#ifdef trayhook_EXPORTS
#	define TRAYHOOK_API __declspec(dllexport)
#else
#	define TRAYHOOK_API __declspec(dllimport)
#endif

struct TrayItemData
{
	COPYDATASTRUCT m_cds;
	union {
		NOTIFYICONDATA m_nic;
		char m_data[1024];
	} m_unic;
};

struct TrayData
{
	mutable bb::SpinLock m_lock;
	size_t m_count;
	std::array<TrayItemData, 32> m_data;

	TrayData () : m_count(0) { }

	bool AddOrModify (UINT uID, HWND hwnd, NOTIFYICONDATA const * nic, COPYDATASTRUCT const * cds)
	{
		m_lock.Lock();
		bool found = false;
		bool ok = true;
		size_t idx = 0;
		for (size_t i = 0, ie = m_count; i < ie; ++i)
		{
			TrayItemData const & d = m_data[i];
			if (d.m_unic.m_nic.uID == uID && d.m_unic.m_nic.hWnd == hwnd)
			{
				idx = i;
				found = true;
				break;
			}
		}
		if (found)
		{
			TrayItemData & d = m_data[idx];
			memset(&d, 0, sizeof(TrayItemData));
			memcpy(&d.m_cds, cds, sizeof(COPYDATASTRUCT));
			memcpy(d.m_unic.m_data, nic, sizeof(NOTIFYICONDATA));
		}
		else
		{
			if (m_count >= m_data.size())
				ok = false;
			else
			{
				size_t const curr = m_count++;
				TrayItemData & d = m_data[curr];
				memset(&d, 0, sizeof(TrayItemData));
				memcpy(&d.m_cds, cds, sizeof(COPYDATASTRUCT));
				memcpy(d.m_unic.m_data, nic, sizeof(NOTIFYICONDATA));
			}
		}
		m_lock.Unlock();
		return ok;
	}

	void DoRemove (size_t idx, size_t & end)
	{
		std::swap(m_data[idx], m_data[end - 1]);
		--end;
	}

	bool Delete (UINT uID, HWND hwnd)
	{
		m_lock.Lock();
		bool found = false;
		for (size_t i = 0, ie = m_count; i < ie; ++i)
		{
			TrayItemData const & d = m_data[i];
			if (d.m_unic.m_nic.uID == uID && d.m_unic.m_nic.hWnd == hwnd)
			{
				DoRemove(i, m_count);
				found = true;
				break;
			}
		}
		m_lock.Unlock();
		return found;
	}
};

//extern alignas(128) TRAYHOOK_API char g_trayDataRaw[sizeof(TrayData)];
extern TRAYHOOK_API TrayData * g_trayData;
extern TRAYHOOK_API HHOOK g_trayHook;

TRAYHOOK_API void initTrayHook (HWND bb_hwnd);
TRAYHOOK_API void doneTrayHook ();

