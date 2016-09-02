#pragma once
#include <vector>
#include <guiddef.h>
#include <bblib/bbstring.h>
#include <platform_win.h>
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
		std::vector<GUID> m_desktops;
		std::vector<bbstring> m_names;
		std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> m_edges;

		bool GetAdjacentDesktop (GUID desk, AdjacentDesktop const & dir, GUID & adj_desk);
		bool GetCurrentDesktop (GUID & adj_desk);
		void UpdateDesktopGraph ();
		bool FindDesktop (bbstring const & name, size_t & idx);
		bool FindDesktop (GUID const & guid, size_t & idx);
		bool FindDesktopIndex (HWND hwnd, size_t & idx);
		bool SwitchDesktop (GUID const & g);
		bool MoveWindowToDesktop (HWND hwnd, GUID const & guid);
	};

}
