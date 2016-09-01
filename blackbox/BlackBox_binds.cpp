#include "BlackBox.h"
#include <bblib/utils_paths.h>
#include "utils_window.h"

namespace bb {

	void BlackBox::Quit (uint32_t arg)
	{
	}

	void BlackBox::MakeSticky (HWND hwnd)
	{
	}

	void BlackBox::RemoveSticky (HWND hwnd)
	{
	}

	HWND BlackBox::GetHwnd ()
	{
		return m_hwnd;
	}

	void * BlackBox::GetSettingPtr (int sn_index)
	{
		if (m_style)
		{
			void * const ptr = m_style->GetStyleMemberPtr(sn_index);
			return ptr;
		}
		else
		{
			return m_defaultStyle->GetStyleMemberPtr(sn_index);
		}
	}

	bool BlackBox::GetConfigDir (wchar_t * dir, size_t dir_sz) const
	{
		wchar_t name[MAX_PATH];
		if (!m_cmdLine.ConfigDir().empty())
		{
			// conversion utf8->utf16
			return true;
		}
		else if (getExeName(name, MAX_PATH))
		{
			return true;
		}
		return false;
	}

	bool BlackBox::WorkSpacesSetCurrentVertexId (bbstring const & vertex_id)
	{
		if (bbstring const * curr_vtx_id = m_wspaces.GetCurrentVertexId())
		{
			bbstring const curr_ws = *curr_vtx_id;
			if (m_wspaces.CanSetCurrentVertexId(vertex_id))
			{
				m_wspaces.SetCurrentVertexId(vertex_id);
				m_tasks.SwitchWorkSpace(curr_ws, vertex_id);
			}
		}
		return false;
	}

	bool BlackBox::WorkSpacesSwitchVertexViaEdge (bbstring const & edge_property)
	{
		bbstring new_vertex_id;
		if (FindTargetVertexViaEdge(edge_property, new_vertex_id))
			if (bbstring const * curr_vtx_id = m_wspaces.GetCurrentVertexId())
			{
				bbstring const curr_ws = *curr_vtx_id;
				m_wspaces.SetCurrentVertexId(new_vertex_id);
				m_tasks.SwitchWorkSpace(curr_ws, new_vertex_id);
				return true;
			}
		return false;
	}

	void BlackBox::MaximizeTopWindow (bool vertical)
	{
		if (HWND hwnd = FindTopLevelWindow())
			maximizeWindow(hwnd, vertical);
	}

	bool BlackBox::FindTargetVertexViaEdge (bbstring const & edge_property, bbstring & dst_vertex_id) const
	{
		if (m_wspaces.CanSwitchVertexViaEdge(edge_property, dst_vertex_id))
			return true;
		return false;
	}
	bool BlackBox::MoveTopWindowToVertexViaEdge (bbstring const & edge_property)
	{
		bbstring new_vertex_id;
		if (HWND hwnd = FindTopLevelWindow())
			if (FindTargetVertexViaEdge(edge_property, new_vertex_id))
				if (bbstring const * curr_vtx_id = m_wspaces.GetCurrentVertexId())
				{
					bbstring const curr_ws = *curr_vtx_id;
					m_tasks.MoveWindowToVertex(hwnd, new_vertex_id);
					return true;
				}
		return false;
	}

}

