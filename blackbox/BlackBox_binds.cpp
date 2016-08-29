#include "BlackBox.h"
#include <bblib/utils_paths.h>

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
		WorkSpaces & ws = m_wspaces;

		bbstring const & cluster_id = ws.GetCurrentClusterId();
		if (WorkGraphConfig const * wg = ws.FindCluster(cluster_id))
		{
			bbstring const & curr_ws = wg->m_currentVertexId;
			if (ws.CanSetCurrentVertexId(vertex_id))
			{
				ws.SetCurrentVertexId(vertex_id);
				m_tasks.SwitchWorkSpace(curr_ws, vertex_id);
			}
		}
		return false;
	}

	bool BlackBox::WorkSpacesSwitchVertexViaEdge (bbstring const & edge_property)
	{
		WorkSpaces & ws = m_wspaces;

		bbstring const & cluster_id = ws.GetCurrentClusterId();
		if (WorkGraphConfig const * wg = ws.FindCluster(cluster_id))
		{
			bbstring const & curr_ws = wg->m_currentVertexId;

			bbstring new_vertex_id;
			if (ws.CanSwitchVertexViaEdge(edge_property, new_vertex_id))
			{
				ws.SetCurrentVertexId(new_vertex_id);
				m_tasks.SwitchWorkSpace(curr_ws, new_vertex_id);
				return true;
			}
		}
		return false;
	}

}

