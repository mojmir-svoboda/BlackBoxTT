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

		tasks.m_lock.Lock();
		for (Tasks::TaskInfoPtr & t : tasks.m_tasks)
			if (t)
				m_tasks.emplace_back(t);
		for (Tasks::TaskInfoPtr & t : tasks.m_otherWS)
			if (t)
				m_tasks.emplace_back(t);
		tasks.m_lock.Unlock();
	}

	void PagerWidget::DrawUI ()
	{
		WorkSpaces const & ws = BlackBox::Instance().GetWorkSpaces();

		UpdateTasks();

		bbstring const & cluster_id = ws.GetCurrentClusterId();
		if (WorkGraphConfig const * wg = ws.FindCluster(cluster_id))
		{
			ImGui::Text("Cluster: %s", cluster_id.c_str());

			uint32_t const rows = wg->MaxRowCount();
			uint32_t const cols = wg->MaxColCount();

			ImGui::Columns(cols, "mixed");
			ImGui::Separator();

			for (uint32_t c = 0; c < cols; ++c)
			{
				for (uint32_t r = 0; r < rows; ++r)
				{
					bbstring const & vertex_id = wg->m_vertexlists[r][c];
					std::string idu8;
					codecvt_utf16_utf8(vertex_id, idu8); // @TODO: perf!

					if (ImGui::Button(idu8.c_str()))
					{ 
						BlackBox::Instance().GetWorkSpaces().SetCurrentVertexId(vertex_id);
					}

					int tmp = 0;
					for (PagerTaskInfo & t : m_tasks)
					{
						if (tmp++ % 4)
							ImGui::SameLine();
						if (t.m_exclude)
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

