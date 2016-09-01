#include "VirtualDesktopManager.h"
#include "utils_vdm.h"
#include <bblib/ScopeGuard.h>
#include <memory>
#include <functional>

/*
MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
		__RPC__in HWND topLevelWindow,
		__RPC__out BOOL *onCurrentDesktop) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
		__RPC__in HWND topLevelWindow,
		__RPC__out GUID *desktopId) = 0;

	virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
		__RPC__in HWND topLevelWindow,
		__RPC__in REFGUID desktopId) = 0;
};
*/


// void PrintGuid(const GUID &guid)
// {
// 	std::wstring guidStr(40, L'\0');
// 	::StringFromGUID2(guid, const_cast<LPOLESTR>(guidStr.c_str()), guidStr.length());
// }


/*
{
	IVirtualDesktop *pNewDesktop = nullptr;
	hr = pDesktopManager->CreateDesktopW(&pNewDesktop);

	if (SUCCEEDED(hr))
	{
		GUID id;
		hr = pNewDesktop->GetID(&id);

		hr = pDesktopManager->SwitchDesktop(pNewDesktop);
		hr = pDesktopManager->RemoveDesktop(pNewDesktop, pDesktop);
	}
}*/

namespace bb {

	VirtualDesktopManager::VirtualDesktopManager () { }

	bool VirtualDesktopManager::Init (size_t l, size_t r)
	{
		m_left = l;
		m_right = r;

		IServiceProvider * isvc = nullptr;
		if (!SUCCEEDED(::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&isvc)))
			return false;

		scope_guard_t on_exit_isvc = mkScopeGuard(std::mem_fun(&IServiceProvider::Release), isvc);

		IVirtualDesktopManager * ivdm = nullptr;
		if (!SUCCEEDED(isvc->QueryService(__uuidof(IVirtualDesktopManager), &ivdm)))
			return false;

		IVirtualDesktopManagerInternal * ivdmi = nullptr;
		if (!SUCCEEDED(isvc->QueryService(CLSID_VirtualDesktopAPI_Unknown, &ivdmi)))
			return false;

		m_vdm = ivdm;
		m_vdmi = ivdmi;

		UpdateDesktopGraph();
		return true;
	}

	bool VirtualDesktopManager::Done ()
	{
		m_desktops.clear();
		m_names.clear();
		m_edges.clear();

		if (m_vdmi)
		{
			m_vdmi->Release();
			m_vdmi = nullptr;
		}
		if (m_vdm)
		{
			m_vdm->Release();
			m_vdm = nullptr;
		}
		return true;
	}

	bool VirtualDesktopManager::SwitchDesktop (GUID const & g)
	{
		IVirtualDesktop * ivd = nullptr;
		GUID g0 = g;
		if (SUCCEEDED(m_vdmi->FindDesktop(&g0, &ivd)))
		{
			scope_guard_t on_exit_ivd = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd);
			if (SUCCEEDED(m_vdmi->SwitchDesktop(ivd)))
				return true;
		}
		return false;
	}

	void VirtualDesktopManager::UpdateDesktopGraph ()
	{
		m_desktops.clear();
		m_names.clear();
		m_edges.clear();

		IObjectArray * iobjarr = nullptr;
		if (SUCCEEDED(m_vdmi->GetDesktops(&iobjarr)))
		{
			scope_guard_t on_exit_iobjarr = mkScopeGuard(std::mem_fun(&IObjectArray::Release), iobjarr);

			UINT count = 0;
			if (SUCCEEDED(iobjarr->GetCount(&count)))
			{
				for (UINT i = 0; i < count; i++)
				{
					IVirtualDesktop * ivd = nullptr;
					if (FAILED(iobjarr->GetAt(i, __uuidof(IVirtualDesktop), (void**)&ivd)))
						continue;
					scope_guard_t on_exit_ivd = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd);

					GUID id = { 0 };
					if (SUCCEEDED(ivd->GetID(&id)))
					{
						m_desktops.push_back(id);
						bbstring name(L"Desktop ");
						name += std::to_wstring(m_desktops.size());
						m_names.push_back(name);
					}
				}
			}
		}

		for (size_t i = 0, ie = m_desktops.size(); i < ie; ++i)
		{
			GUID const & g0 = m_desktops[i];
			GUID g1;
			size_t j = 0;
			if (GetAdjacentDesktop(g0, AdjacentDesktop::LeftDirection, g1))
				if (FindDesktop(g1, j))
					m_edges.push_back(std::make_tuple(i, m_left, j));
			if (GetAdjacentDesktop(g0, AdjacentDesktop::RightDirection, g1))
				if (FindDesktop(g1, j))
					m_edges.push_back(std::make_tuple(i, m_right, j));
		}
	}

	bool VirtualDesktopManager::FindDesktop (bbstring const & name, size_t & idx)
	{
		for (size_t i = 0, ie = m_names.size(); i < ie; ++i)
		{
			if (m_names[i] == name)
			{
				idx = i;
				return true;
			}
		}
		return false;
	}
	bool VirtualDesktopManager::FindDesktop (GUID const & guid, size_t & idx)
	{
		for (size_t i = 0, ie = m_names.size(); i < ie; ++i)
		{
			if (m_desktops[i] == guid)
			{
				idx = i;
				return true;
			}
		}
		return false;
	}

	bool VirtualDesktopManager::GetCurrentDesktop (GUID & desk)
	{
		IVirtualDesktop * ivd = nullptr;
		if (SUCCEEDED(m_vdmi->GetCurrentDesktop(&ivd)))
		{
			scope_guard_t on_exit_ivd = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd);

			GUID id = { 0 };
			if (SUCCEEDED(ivd->GetID(&id)))
			{
				desk = id;
				return true;
			}
		}
		return false;
	}

	bool VirtualDesktopManager::GetAdjacentDesktop (GUID desk, AdjacentDesktop const & dir, GUID & adj_desk)
	{
		IVirtualDesktop * ivd_curr = nullptr;
		if (SUCCEEDED(m_vdmi->FindDesktop(&desk, &ivd_curr)))
		{
			scope_guard_t on_exit_ivd_curr = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd_curr);

			GUID id = { 0 };
			IVirtualDesktop * ivd_adj = nullptr;
			if (SUCCEEDED(m_vdmi->GetAdjacentDesktop(ivd_curr, dir, &ivd_adj)))
			{
				scope_guard_t on_exit_ivd_adj = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd_adj);

				if (SUCCEEDED(ivd_adj->GetID(&id)))
				{
					adj_desk = id;
					return true;
				}
			}
		}
		return false;
	}

	bool VirtualDesktopManager::FindDesktopIndex (HWND hwnd, size_t & idx)
	{
		GUID g = { 0 };
		if (SUCCEEDED(m_vdm->GetWindowDesktopId(hwnd, &g)))
			return FindDesktop(g, idx);
		return false;
	}

}
