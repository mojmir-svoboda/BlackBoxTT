#include "VirtualDesktopManager.h"
#include <ntverp.h>
#if VER_PRODUCTBUILD > 9600
// Windows 10+ SDK code goes here
#	include "utils_vdm.h"
#	include <blackbox/common.h>
#	include <memory>
#	include <functional>
#	include <BlackBox.h>
// void PrintGuid(const GUID &guid)
// {
// 	::StringFromGUID2(guid, const_cast<LPOLESTR>(guidStr.c_str()), guidStr.length());
// }

namespace bb {

	VirtualDesktopManager::VirtualDesktopManager () { }

	struct VirtualDesktopNotification : IVirtualDesktopNotification
	{
		VirtualDesktopManager & m_vdm;
		VirtualDesktopNotification (VirtualDesktopManager & vdm) : m_vdm(vdm) { }
		virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated (IVirtualDesktop * pDesktop) override;
		virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) override;
		virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) override;
		virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) override;
		virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged (IApplicationView * pView) override;
		virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged (IVirtualDesktop * pDesktopOld, IVirtualDesktop * pDesktopNew) override;

		ULONG m_count { 0 };
		virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void ** ppvObject) override
		{
			if (!ppvObject)
				return E_INVALIDARG;
			*ppvObject = nullptr;

			if (riid == IID_IUnknown || riid == IID_IVirtualDesktopNotification)
			{
				// Increment the reference count and return the pointer.
				*ppvObject = (LPVOID)this;
				AddRef();
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		virtual ULONG STDMETHODCALLTYPE AddRef () override
		{
			return InterlockedIncrement(&m_count);
		}

		virtual ULONG STDMETHODCALLTYPE Release () override
		{
			ULONG result = InterlockedDecrement(&m_count);
			if (result == 0)
				delete this;
			return result;
		}

	};

	HRESULT VirtualDesktopNotification::VirtualDesktopCreated (IVirtualDesktop * pDesktop)
	{
		bb::BlackBox::Instance().GetWorkSpaces().OnGraphConfigurationChanged();
		return S_OK;
	}
	HRESULT VirtualDesktopNotification::VirtualDesktopDestroyBegin (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback)
	{
		return S_OK;
	}
	HRESULT VirtualDesktopNotification::VirtualDesktopDestroyFailed (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback)
	{
		return S_OK;
	}
	HRESULT VirtualDesktopNotification::VirtualDesktopDestroyed (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback)
	{
		bb::BlackBox::Instance().GetWorkSpaces().OnGraphConfigurationChanged();
		return S_OK;
	}
	HRESULT VirtualDesktopNotification::ViewVirtualDesktopChanged (IApplicationView * pView)
	{
		return S_OK;
	}
	HRESULT VirtualDesktopNotification::CurrentVirtualDesktopChanged (IVirtualDesktop * pDesktopOld, IVirtualDesktop * pDesktopNew)
	{
		// @TODO @PERF @FIXME .. how to get guid from IVirtualDesktp?
		IVirtualDesktop * pd[2] = { pDesktopOld, pDesktopNew };
		size_t idx[2] = { 0 };
		size_t found = 0;
		for (size_t i = 0; i < 2; ++i)
		{
			for (size_t j = 0, je = m_vdm.m_desktops.size(); j < je; ++j)
			{
				IVirtualDesktop * ivd = nullptr;
				GUID g = { 0 };
				if (SUCCEEDED(m_vdm.m_vdmi->FindDesktop(&m_vdm.m_desktops[j], &ivd)))
				{
					scope_guard_t on_exit_ivd = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd);
					if (ivd == pd[i])
					{
						idx[i] = j;
						++found;
					}
				}
				else
					return S_FALSE;
			}
		}
		if (found == 2)
		{
			bb::BlackBox::Instance().GetTasks().OnSwitchDesktopVDM(m_vdm.m_ids[idx[0]], m_vdm.m_ids[idx[1]]);
			bb::BlackBox::Instance().GetWorkSpaces().OnSwitchedDesktop();
		}
		return S_OK;
	}

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
		scope_guard_t on_exit_ivdm = mkScopeGuard(std::mem_fun(&IVirtualDesktopManager::Release), ivdm);

		// trying versions... thank you, M$
		IVirtualDesktopManagerInternal10130 * ivdmi10130 = nullptr;
		IVirtualDesktopManagerInternal10240 * ivdmi10240 = nullptr;
		IVirtualDesktopManagerInternal10536 * ivdmi10536 = nullptr;
		if (!SUCCEEDED(isvc->QueryService(CLSID_VirtualDesktopAPI_Unknown, &ivdmi10130)))
			if (!SUCCEEDED(isvc->QueryService(CLSID_VirtualDesktopAPI_Unknown, &ivdmi10240)))
				if (!SUCCEEDED(isvc->QueryService(CLSID_VirtualDesktopAPI_Unknown, &ivdmi10536)))
					return false;
		IVirtualDesktopManagerInternal * ivdmi = nullptr;
		if (ivdmi10130)
			ivdmi = ivdmi10130;
		if (ivdmi10240)
			ivdmi = ivdmi10240;
		if (ivdmi10536)
			ivdmi = ivdmi10536;
		scope_guard_t on_exit_ivdmi = mkScopeGuard(std::mem_fun(&IVirtualDesktopManagerInternal::Release), ivdmi);

		IVirtualDesktopNotificationService * notif_svc = nullptr;
		if (!SUCCEEDED(isvc->QueryService(CLSID_IVirtualNotificationService, &notif_svc)))
			return false;
		scope_guard_t on_exit_notif_svc = mkScopeGuard(std::mem_fun(&IVirtualDesktopNotificationService::Release), notif_svc);

		DWORD cookie = 0;
		m_notif = new VirtualDesktopNotification(*this);
		if (!SUCCEEDED(notif_svc->Register(m_notif, &cookie)))
			return false;

		IApplicationViewCollection * iavc = nullptr;
		if (!SUCCEEDED(isvc->QueryService(IID_IApplicationViewCollection, IID_IApplicationViewCollection, (PVOID*)&iavc)))
			return false;
		scope_guard_t on_exit_iavc = mkScopeGuard(std::mem_fun(&IApplicationViewCollection::Release), iavc);

		IVirtualDesktopPinnedApps * ivdpa = nullptr;
		if (!SUCCEEDED(isvc->QueryService(CLSID_VirtualDesktopPinnedApps, __uuidof(IVirtualDesktopPinnedApps), (PVOID*)&ivdpa)))
		{ 
// can fail on 10240
//			return false;
		}
//		scope_guard_t on_exit_ivdpa = mkScopeGuard(std::mem_fun(&IVirtualDesktopPinnedApps::Release), ivdpa);

		on_exit_ivdm.Dismiss();
		on_exit_ivdmi.Dismiss();
		on_exit_iavc.Dismiss();
//		on_exit_ivdpa.Dismiss();

		m_vdm = ivdm;
		m_vdmi = ivdmi;
		m_avc = iavc;
		m_notif_cookie = cookie;
		m_vdpa = ivdpa;

		UpdateDesktopGraph();
		return true;
	}

	bool VirtualDesktopManager::Done ()
	{
		if (m_notif_cookie != 0)
		{
			IServiceProvider * isvc = nullptr;
			if (SUCCEEDED(::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&isvc)))
			{
				scope_guard_t on_exit_isvc = mkScopeGuard(std::mem_fun(&IServiceProvider::Release), isvc);
				IVirtualDesktopNotificationService * notif_svc = nullptr;
				if (SUCCEEDED(isvc->QueryService(CLSID_IVirtualNotificationService, &notif_svc)))
				{
					scope_guard_t on_exit_notif_svc = mkScopeGuard(std::mem_fun(&IVirtualDesktopNotificationService::Release), notif_svc);
					if (!SUCCEEDED(notif_svc->Unregister(m_notif_cookie)))
						TRACE_MSG(LL_ERROR, CTX_WSPACE, "Cannot unregister vdm notification");
				}
			}
		}
		if (m_notif)
		{
			delete m_notif;
			m_notif = nullptr;
		}
		m_desktops.clear();
		m_names.clear();
		m_edges.clear();

		if (m_avc)
		{
			m_avc->Release();
			m_avc = nullptr;
		}
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
		if (m_vdpa)
		{
			m_vdpa->Release();
			m_vdpa = nullptr;
		}
		return true;
	}

	bool VirtualDesktopManager::MoveWindowToDesktop (HWND hwnd, GUID const & guid)
	{
		IVirtualDesktop * ivd = nullptr;
		GUID g0 = guid;
		if (SUCCEEDED(m_vdmi->FindDesktop(&g0, &ivd)))
		{
			scope_guard_t on_exit_ivd = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd);
			IApplicationView * iav = nullptr;
			if (SUCCEEDED(m_avc->GetViewForHwnd(hwnd, &iav)))
			{
				scope_guard_t on_exit_iav = mkScopeGuard(std::mem_fun(&IApplicationView::Release), iav);
				if (SUCCEEDED(m_vdmi->MoveViewToDesktop(iav, ivd)))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool VirtualDesktopManager::SupportsPinnedViews () const
	{
		return m_vdpa && m_avc;
	}

	bool VirtualDesktopManager::IsPinned (HWND hwnd) const
	{
		if (SupportsPinnedViews())
		{
			IApplicationView * v = nullptr;
			if (SUCCEEDED(m_avc->GetViewForHwnd(hwnd, &v)))
			{
				BOOL b;
				m_vdpa->IsViewPinned(v, &b);
				if (b == TRUE)
					return true;
			}
		}
		return false;
	}

	bool VirtualDesktopManager::SetPinned (HWND hwnd, bool on) const
	{
		if (SupportsPinnedViews())
		{
			IApplicationView * v = nullptr;
			if (SUCCEEDED(m_avc->GetViewForHwnd(hwnd, &v)))
			{
				if (on)
					m_vdpa->PinView(v);
				else
					m_vdpa->UnpinView(v);
				return true;
			}
		}
		return false;

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
		m_ids.clear();
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
		for (size_t i = 0, ie = m_ids.size(); i < ie; ++i)
		{
			if (m_ids[i] == name)
			{
				idx = i;
				return true;
			}
		}
		return false;
	}
	bool VirtualDesktopManager::FindDesktop (GUID const & guid, size_t & idx)
	{
		for (size_t i = 0, ie = m_desktops.size(); i < ie; ++i)
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

	void VirtualDesktopManager::ClearAssignedDesktops ()
	{
		m_ids.clear();
	}

	bool VirtualDesktopManager::CreateDesktops (size_t needed_total_count)
	{
		bool failed = false;
		size_t const n = m_desktops.size();
		if (n < needed_total_count)
		{
			for (size_t i = n; i < needed_total_count; ++i)
			{
				IVirtualDesktop * ivd = nullptr;
				if (SUCCEEDED(m_vdmi->CreateDesktopW(&ivd)))
				{
					scope_guard_t on_exit_ivd = mkScopeGuard(std::mem_fun(&IVirtualDesktop::Release), ivd);
				}
				else
					failed = true;
			}
		}
		return failed;
	}

	bool VirtualDesktopManager::AssignDesktopTo (bbstring const & vertex_id, size_t & idx)
	{
		if (m_ids.size() < m_desktops.size())
		{
			idx = m_ids.size();
			m_ids.push_back(vertex_id);
			return true;
		}
		return false;
	}

}
#else
// Windows 8.1- SDK code goes here

#endif