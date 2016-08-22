#include "PagerWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	PagerWidget::PagerWidget ()
	{
		m_tasks.reserve(64);
	}

	void PagerWidget::UpdateTasks ()
	{
		m_tasks.clear();

		Tasks & tasks = BlackBox::Instance().GetTasks();
		tasks.MkDataCopy(e_Active, m_tasks);
		tasks.MkDataCopy(e_OtherWS, m_tasks);
	}

	void PagerWidget::DrawUI ()
	{
		WorkSpaces const & ws = BlackBox::Instance().GetWorkSpaces();

		UpdateTasks();

		bbstring const & cluster_id = ws.GetCurrentClusterId();
		if (WorkGraphConfig const * wg = ws.FindCluster(cluster_id))
		{
			char curr_ws_u8[TaskInfo::e_wspaceLenMax];
			codecvt_utf16_utf8(wg->m_currentVertexId, curr_ws_u8, TaskInfo::e_wspaceLenMax);

			ImGui::Text("%s %s", cluster_id.c_str(), curr_ws_u8);

			uint32_t const rows = wg->MaxRowCount();
			uint32_t const cols = wg->MaxColCount();

			ImGui::Columns(cols, "mixed");
			ImGui::Separator();

			for (uint32_t c = 0; c < cols; ++c)
			{
				for (uint32_t r = 0; r < rows; ++r)
				{
					bbstring const & vertex_id = wg->m_vertexlists[r][c];
					char idu8[TaskInfo::e_wspaceLenMax];
					codecvt_utf16_utf8(vertex_id, idu8, TaskInfo::e_wspaceLenMax);

					if (ImGui::Button(idu8))
					{ 
						BlackBox::Instance().WorkSpacesSetCurrentVertexId(vertex_id);
					}

					int tmp = 0;
					for (TaskInfo & t : m_tasks)
					{
						if (++tmp % 3)
							ImGui::SameLine();
						if (t.m_config && t.m_config->m_exclude)
							continue;

						if (t.m_wspace == vertex_id)
						{
							IconId const icoid = t.m_icoSmall;
							ImGui::Icon(icoid, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
							if (!icoid.IsValid())
							{
								// @TODO: assign color to hwnd?
								ImGui::ColorButton(ImColor(0, 0, 128, 255));
							}
						}
					}
				}
				ImGui::NextColumn();
			}
		}
	}

}

