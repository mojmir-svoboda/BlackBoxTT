#include "PagerWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <bblib/codecvt.h>
#include <imgui/imgui_internal.h>
#include <blackbox/utils_window.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"

namespace YAML {
	template<>
	struct convert<bb::imgui::PagerWidgetConfig>
	{
		static Node encode(bb::imgui::PagerWidgetConfig const & rhs)
		{
			Node node = convert<bb::WidgetConfig>::encode(rhs);
			//node.push_back(rhs.);
			return node;
		}

		static bool decode (Node const & node, bb::imgui::PagerWidgetConfig & rhs)
		{
			try
			{
				if (convert<bb::WidgetConfig>::decode(node, rhs))
				{
					return true;
				}
			}
			catch (std::exception const & e)
			{
				return false;
			}
			return true;
		}
	};
}

namespace bb {
namespace imgui {

	PagerWidget::PagerWidget ()
		: GuiWidget()
	{
		m_tasks.reserve(64);
	}

	bool PagerWidget::loadConfig (YAML::Node & y_cfg_node)
	{
		if (!y_cfg_node.IsNull())
		{
			PagerWidgetConfig tmp = y_cfg_node.as<PagerWidgetConfig>();
			m_config = std::move(tmp);
			return true;
		}
		return false;
	}

	void PagerWidget::UpdateTasks ()
	{
		m_tasks.clear();

		Tasks & tasks = BlackBox::Instance().GetTasks();
		tasks.MkDataCopy(m_tasks);
	}

	void PagerWidget::DrawUI ()
	{
		WorkSpaces const & ws = BlackBox::Instance().GetWorkSpaces();
		char title[256];
		bbstring const & cluster_id = ws.GetCurrentClusterId();
		WorkGraphConfig const * wg = ws.FindCluster(cluster_id);
		if (wg == nullptr)
			return;

		char curr_ws_u8[TaskInfo::e_wspaceLenMax];
		codecvt_utf16_utf8(wg->m_currentVertexId, curr_ws_u8, TaskInfo::e_wspaceLenMax);
		char id_u8[128];
		codecvt_utf16_utf8(GetId().c_str(), id_u8, 128);
		char cluster_u8[128];
		codecvt_utf16_utf8(cluster_id.c_str(), cluster_u8, 128);
		_snprintf(title, 256, "%s (%s)###%s", curr_ws_u8, cluster_u8, id_u8);

		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;
		if (m_contentSize.x > 0 && m_contentSize.y > 0)
		{
			ImGuiStyle & style = ImGui::GetStyle();
			resizeWindowToContents(m_gfxWindow->m_hwnd, m_contentSize.x, m_contentSize.y, style.WindowMinSize.x, style.WindowMinSize.y, style.WindowRounding);
		}

		ImGui::Begin(title, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

		UpdateTasks();

		uint32_t const rows = wg->MaxRowCount();
		uint32_t const cols = wg->MaxColCount();

		if (cols && rows)
		{
			ImGui::Columns(cols, "mixed", true);
			for (uint32_t r = 0; r < rows; ++r)
			{
				for (uint32_t c = 0; c < cols; ++c)
				{
					bbstring const & vertex_id = wg->m_vertexlists[r][c];
					char idu8[TaskInfo::e_wspaceLenMax];
					codecvt_utf16_utf8(vertex_id, idu8, TaskInfo::e_wspaceLenMax);

					if (ImGui::Button(idu8))
					{ 
						BlackBox::Instance().WorkSpacesSetCurrentVertexId(vertex_id);
					}

					if (m_dragged && ImGui::IsItemHoveredRect() && ImGui::IsMouseReleased(0))
					{
						// dropped item
						HWND const hwnd = m_dragged;
						m_dragged = nullptr;
						BlackBox::Instance().MoveWindowToVertex(hwnd, vertex_id);
					}

					int tmp = 0;
					for (TaskInfo & t : m_tasks)
					{
// 						if (t.m_config && t.m_config->m_bbtasks)
// 							continue;
						if (t.m_wspace == vertex_id || t.IsSticky())
						{
							if (tmp++ % 3 != 0)
								ImGui::SameLine();

							Tasks & tasks = BlackBox::Instance().GetTasks();
							IconId const icoid = t.m_icoSmall;
							if (!icoid.IsValid())
							{
								// @TODO: assign color to hwnd?
								if (ImGui::ColorButton(ImColor(0, 0, 128, 255)))
								{
									tasks.Focus(t.m_hwnd);
								}
							}
							else
							{
								int framing = -1;
								ImGui::PushID(t.m_hwnd);
								bool const skip_taskman = t.IsTaskManIgnored();
								bool const is_sticky = t.IsSticky();
								ImColor const col_int = ImColor(0, 0, 0, 0);
								ImColor const col_border = ImColor(255, 255, 255, 255);
								ImColor const col_int_skip_taskman = ImColor(0, 0, 0, 128);
								ImColor const col_border_skip = ImColor(0, 0, 0, 128);
								bool const clkd = ImGui::IconButton(icoid, skip_taskman ? col_int_skip_taskman : col_int, skip_taskman ? col_border_skip : col_border, framing);

								if (ImGui::IsMouseDragging() && ImGui::IsItemActive())
								{
									if (!m_dragged)
									{
										m_dragged = t.m_hwnd;
									}
									void * texid = nullptr;
									float u0 = 0.0f;
									float v0 = 0.0f;
									float u1 = 0.0f;
									float v1 = 0.0f;
									/// @TODO: @NOTE: PERFORMANCE !!!
									if (bb::BlackBox::Instance().GetGfx().FindIconCoords(icoid, texid, u0, v0, u1, v1))
									{
										ImDrawList* draw_list = ImGui::GetWindowDrawList();
										draw_list->PushClipRectFullScreen();
										ImVec2 const mp = ImGui::GetIO().MousePos;
										ImVec2 const p1(mp.x - icoid.m_size / 2, mp.y - icoid.m_size / 2);
										ImVec2 const p2(mp.x + icoid.m_size / 2, mp.y + icoid.m_size / 2);
										ImVec2 const rp1(p1.x - 1, p1.y - 1);
										ImVec2 const rp2(p2.x - 1, p2.y - 1);
										draw_list->AddImage(texid, p1, p2, ImVec2(u0, v0), ImVec2(u1, v1), col_border);
										draw_list->AddRect(rp1, rp2, col_border);
										draw_list->PopClipRect();
									}
								}

								if (ImGui::BeginPopupContextItem(""))
								{
									if (ImGui::Selectable(is_sticky ? "UnStick" : "Stick"))
									{
										if (is_sticky)
											tasks.UnsetSticky(t.m_hwnd);
										else
											tasks.SetSticky(t.m_hwnd);
									}

									if (ImGui::Selectable(skip_taskman ? "back to TaskMan" : "rm from TaskMan"))
									{
										if (skip_taskman)
											tasks.UnsetTaskManIgnored(t.m_hwnd);
										else
											tasks.SetTaskManIgnored(t.m_hwnd);
									}

									if (ImGui::Button("Close Menu"))
										ImGui::CloseCurrentPopup();
									ImGui::EndPopup();
								}
								if (clkd)
								{
									tasks.Focus(t.m_hwnd);
								}
								ImGui::PopID();
							}
						}
					}
					ImGui::NextColumn();
				}
				ImGui::Separator();
			}
		}
		ImGuiWindow * w = ImGui::GetCurrentWindowRead();
		ImVec2 const & sz1 = w->SizeContents;
		m_contentSize = sz1;
		ImGui::End();
 	}

}}

