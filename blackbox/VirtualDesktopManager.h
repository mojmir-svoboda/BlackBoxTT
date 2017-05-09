#pragma once
#include <ntverp.h>
#include <common.h>
#if VER_PRODUCTBUILD > 9600
#include <vector>
#include <guiddef.h>
struct IVirtualDesktopManagerInternal;
struct IVirtualDesktopManager;
struct IApplicationViewCollection;
struct IVirtualDesktopPinnedApps;
enum AdjacentDesktop
{
	LeftDirection = 3,
	RightDirection = 4
};

namespace bb {
	struct VirtualDesktopNotification;

	struct VirtualDesktopManager
	{
		VirtualDesktopManager ();
		uint32_t m_left { 0 }; // shared properies between bb and vdm
		uint32_t m_right { 0 };
		bool Init (size_t l, size_t r);
		bool Done ();

	protected:
		friend struct WorkSpaces;
		friend class Tasks;
		friend struct VirtualDesktopNotification;
		IVirtualDesktopManagerInternal * m_vdmi { nullptr };
		IVirtualDesktopManager * m_vdm { nullptr };
		IApplicationViewCollection * m_avc { nullptr };
		unsigned m_notif_cookie { 0 };
		VirtualDesktopNotification * m_notif { nullptr };
		IVirtualDesktopPinnedApps * m_vdpa { nullptr };
		std::vector<GUID> m_desktops;
		std::vector<bbstring> m_ids; /// assigned vertex ids to GUIDs
		std::vector<bbstring> m_names;
		std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> m_edges;

		bool GetAdjacentDesktop (GUID desk, AdjacentDesktop const & dir, GUID & adj_desk);
		bool GetCurrentDesktop (GUID & adj_desk);
		void UpdateDesktopGraph ();
		bool FindDesktop (bbstring const & name, size_t & idx);
		bool FindDesktop (GUID const & guid, size_t & idx);
		bool FindDesktopIndex (HWND hwnd, size_t & idx);
		bool AssignDesktopTo (bbstring const & vertex_id, size_t & idx);
		bool CreateDesktops (size_t needed_total_count);
		size_t GetDesktopCount () const { return m_desktops.size(); }
		void ClearAssignedDesktops ();
		bool SwitchDesktop (GUID const & g);
		bool MoveWindowToDesktop (HWND hwnd, GUID const & guid);

		bool SupportsPinnedViews () const;
		bool IsPinned (HWND hwnd) const;
		bool SetPinned (HWND hwnd, bool on) const;
	};

}
#else
namespace bb {

	struct VirtualDesktopManager
	{
		bool Init (size_t l, size_t r) { return true; }
		bool Done () { return true; }
		bool GetAdjacentDesktop (GUID desk, int dir, GUID & adj_desk) { return false; }
		bool GetCurrentDesktop (GUID & adj_desk) { return false; }
		void UpdateDesktopGraph () { }
		bool FindDesktop (bbstring const & name, size_t & idx) { return false; }
		bool FindDesktop (GUID const & guid, size_t & idx) { return false; }
		bool FindDesktopIndex (HWND hwnd, size_t & idx) { return false; }
		bool AssignDesktopTo (bbstring const & vertex_id, size_t & idx) { return false; }
		bool CreateDesktops (size_t needed_total_count) { return false; }
		size_t GetDesktopCount () const { return 0; }
		void ClearAssignedDesktops () { }
		bool SwitchDesktop (GUID const & g) { return false; }
		bool MoveWindowToDesktop (HWND hwnd, GUID const & guid) { return false; }
		bool SupportsPinnedViews () const { return false; }
		bool IsPinned (HWND hwnd) const;
		bool SetPinned (HWND hwnd, bool on) const;
	};
}
#endif