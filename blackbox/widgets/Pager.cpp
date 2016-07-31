#include "Pager.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void PagerWidget::DrawUI ()
	{
		WorkSpaces const & ws = BlackBox::Instance().GetWorkSpaces();
		Tasks & tasks = BlackBox::Instance().GetTasks();

		bbstring const & cluster_id = ws.GetCurrentClusterId();
		if (WorkGraphConfig const * wg = ws.FindCluster(cluster_id))
		{
			ImGui::Text("Cluster: %s", cluster_id.c_str());

			uint32_t const rows = wg->MaxRowCount();
			uint32_t const cols = wg->MaxColCount();

			ImGui::Columns(cols, "mixed");
			ImGui::Separator();

			tasks.m_lock.Lock();
			for (uint32_t c = 0; c < cols; ++c)
			{
				for (uint32_t r = 0; r < rows; ++r)
				{
					bbstring const & vertex_id = wg->m_vertexlists[r][c];
					std::string idu8;
					codecvt_utf16_utf8(vertex_id, idu8); // @TODO: perf!

					if (ImGui::Button(idu8.c_str()))
					{
						// bacha tady sem pod zamkem
						/*switchworkspace*/;
					}

					int tmp = 0;
					for (Tasks::TaskInfoPtr & t : tasks.m_tasks)
					{
						if (++tmp % 4)
							ImGui::SameLine();
						if (t->m_exclude)
							continue;

						if (t->m_wspace == vertex_id)
						{
							IconId const icoid = t->m_icoSmall;
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
			tasks.m_lock.Unlock();
		}
	}

}

