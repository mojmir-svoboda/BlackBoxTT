#pragma once
#include <vector>
#include <guiddef.h>
struct IVirtualDesktopManagerInternal;
struct IVirtualDesktopManager;
enum AdjacentDesktop
{
	LeftDirection = 3,
	RightDirection = 4
};

namespace bb {

	struct VirtualDesktopManager
	{
		VirtualDesktopManager ();
		bool Init ();
		bool Done ();

	protected:
		IVirtualDesktopManagerInternal * m_vdmi { nullptr };
		IVirtualDesktopManager * m_vdm { nullptr };
		std::vector<GUID> m_desktops;
		std::vector<std::pair<GUID, GUID>> m_edges;

		bool GetAdjacentDesktop (GUID desk, AdjacentDesktop const & dir, GUID & adj_desk);
		bool GetCurrentDesktop (GUID & adj_desk);
		void UpdateDesktops ();
	};

}
