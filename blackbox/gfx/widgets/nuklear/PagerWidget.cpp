#include "PagerWidget.h"
#include <blackbox/BlackBox.h>
#include <bblib/codecvt.h>
#include <blackbox/utils_window.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <nuklear/nuklear_config.h>
#include <blackbox/gfx/nuklear/utils_gui.h>

// todle nechci
#include <blackbox/gfx/nuklear/Gui.h>
#include <blackbox/gfx/nuklear/Gfx.h>

namespace YAML {
	template<>
	struct convert<bb::nuklear::PagerWidgetConfig>
	{
		static Node encode(bb::nuklear::PagerWidgetConfig const & rhs)
		{
			Node node = convert<bb::WidgetConfig>::encode(rhs);
			//node.push_back(rhs.);
			return node;
		}

		static bool decode (Node const & node, bb::nuklear::PagerWidgetConfig & rhs)
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
namespace nuklear {

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

	void PagerWidget::ResizePagerColumns ()
	{
// 		WorkSpaces const & ws = BlackBox::Instance().GetWorkSpaces();
// 		bbstring const & cluster_id = ws.GetCurrentClusterId();
// 		WorkGraphConfig const * wg = ws.FindCluster(cluster_id);
// 		if (wg == nullptr)
// 			return;
// 
// 		uint32_t const rows = wg->MaxRowCount();
// 		uint32_t const cols = wg->MaxColCount();
// 
// 		int const spacing = 1;
// 		int * sizes = (int *)alloca(cols * sizeof(int));
// 		for (uint32_t c = 0; c < cols; ++c)
// 		{
// 			sizes[c] = 0;
// 			for (uint32_t r = 0; r < rows; ++r)
// 			{
// 				bbstring const & vertex_id = wg->m_vertexlists[r][c];
// 				for (TaskInfo & t : m_tasks)
// 				{
// 					if (t.m_wspace == vertex_id || t.IsSticky())
// 					{
// 						if (sizes[c] < m_iconsPerRow)
// 							++sizes[c];
// 					}
// 				}
// 			}
// 			if (sizes[c] < 1)
// 				sizes[c] = 1;
// 		}
// 
// 		int n = 0;
// 		ImGui::SetColumnOffset(0, 0);
// 		for (uint32_t c = 0; c < cols; ++c)
// 		{
// 			n += sizes[c];
// 			float const offs = n * 32 + (n - 1) * spacing;
// 			ImGui::SetColumnOffset(c + 1, offs);
// 		}
	}

	void PagerWidget::DrawUI ()
	{
		nk_context * ctx = &((bb::nuklear::Gui *)m_gfxWindow->GetGui())->m_context;
		static struct nk_rect kua = nk_rect(0,0,0,0);
		if (kua.w > 0 && kua.h > 0)
		{
			//ImGuiStyle & style = ImGui::GetStyle();
			resizeWindowToContents(m_gfxWindow->m_hwnd, kua.w, kua.h, 10, 10, 10);
			//resizeWindowToContents(m_gfxWindow->m_hwnd, m_contentSize.x, m_contentSize.y, style.WindowMinSize.x, style.WindowMinSize.y, style.WindowRounding);
		}
#if 0
		if (nk_begin(ctx, "Demo", nk_rect(0, 0, 230, 250),
			/*NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |*/ NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_SCROLL_AUTO_HIDE |
			/*NK_WINDOW_MINIMIZABLE |*/ NK_WINDOW_TITLE))
		{
			enum { EASY, HARD };
			static int op = EASY;
			static int property = 20;

			nk_layout_row_static(ctx, 30, 80, 1);
			if (nk_button_label(ctx, "button"))
				fprintf(stdout, "button pressed\n");
			nk_layout_row_dynamic(ctx, 30, 2);
			if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
			if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
			nk_layout_row_dynamic(ctx, 22, 1);
			nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

			nk_layout_row_dynamic(ctx, 20, 1);
			nk_label(ctx, "background:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_color background = nk_rgb(28,48,62);
			if (nk_combo_begin_color(ctx, background, nk_vec2(nk_widget_width(ctx), 400))) {
				nk_layout_row_dynamic(ctx, 120, 1);
				background = nk_color_picker(ctx, background, NK_RGBA);
				nk_layout_row_dynamic(ctx, 25, 1);
				background.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1, 1);
				background.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1, 1);
				background.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1, 1);
				background.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1, 1);
				nk_combo_end(ctx);
			}
		}
		nk_end(ctx);
#endif

		if (ctx->begin)
		{
			struct nk_rect & sz1 = ctx->begin->bounds;
			kua = sz1;
		}

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
// 
// 		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
// 		ImVec2 const & display = ImGui::GetIO().DisplaySize;
// 		if (m_contentSize.x > 0 && m_contentSize.y > 0)
// 		{
// 			ImGuiStyle & style = ImGui::GetStyle();
// 			resizeWindowToContents(m_gfxWindow->m_hwnd, m_contentSize.x, m_contentSize.y, style.WindowMinSize.x, style.WindowMinSize.y, style.WindowRounding);
// 		}

// 		bool win_opened = true;
// 		if (!ImGui::Begin(title, &win_opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
// 		{
// 			ImGui::End();
// 			return;
// 		}

// 		bool win_opened = true;
// 		if (!ImGui::Begin(title, &win_opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
// 		{
// 			ImGui::End();
// 			return;
// 		}
		if (!nk_begin(ctx, title, nk_rect(0, 0, 230, 250),
			/*NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |*/ NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_SCROLL_AUTO_HIDE |
			/*NK_WINDOW_MINIMIZABLE |*/ NK_WINDOW_TITLE))
		{
			nk_end(ctx);
			return;
		}

// 		if (!win_opened)
// 		{
// 			ImGui::End();
// 			m_gfxWindow->SetDestroyTree();
// 			return;
// 		}
// 
		UpdateTasks();

		uint32_t const rows = wg->MaxRowCount();
		uint32_t const cols = wg->MaxColCount();

		if (cols && rows)
		{
			nk_layout_row_dynamic(ctx, 30, cols);
			//ImGui::Columns(cols, "mixed", true);
			//ResizePagerColumns();

			for (uint32_t r = 0; r < rows; ++r)
			{
				for (uint32_t c = 0; c < cols; ++c)
				{
					bbstring const & vertex_id = wg->m_vertexlists[r][c];
					char idu8[TaskInfo::e_wspaceLenMax];
					codecvt_utf16_utf8(vertex_id, idu8, TaskInfo::e_wspaceLenMax);

					//if (ImGui::Button(idu8))
					if (nk_button_label(ctx, idu8))
					{ 
						BlackBox::Instance().WorkSpacesSetCurrentVertexId(vertex_id);
					}

// 					if (m_dragged && ImGui::IsItemHoveredRect() && ImGui::IsMouseReleased(0))
// 					{
// 						// dropped item
// 						HWND const hwnd = m_dragged;
// 						m_dragged = nullptr;
// 						BlackBox::Instance().MoveWindowToVertex(hwnd, vertex_id);
// 					}

					int tmp = 0;
					for (TaskInfo & t : m_tasks)
					{
//// 						if (t.m_config && t.m_config->m_bbtasks)
//// 							continue;
						if (t.m_wspace == vertex_id || t.IsSticky())
						{
// 							if (tmp++ % m_iconsPerRow != 0)
// 								ImGui::SameLine();

							Tasks & tasks = BlackBox::Instance().GetTasks();
							IconId const icoid = t.m_icoSmall;
							if (!icoid.IsValid())
							{
								// @TODO: assign color to hwnd?
								if (nk_button_color(ctx, struct nk_color { 0, 0, 128, 255 }))
								{
									tasks.Focus(t.m_hwnd);
								}
							}
							else
							{
// 								int framing = -1;
// 								ImGui::PushID(t.m_hwnd);
// 								bool const skip_taskman = t.IsTaskManIgnored();
// 								bool const is_sticky = t.IsSticky();
// 								ImColor const col_int = ImColor(0, 0, 0, 0);
// 								ImColor const col_border = ImColor(255, 255, 255, 255);
// 								ImColor const col_int_skip_taskman = ImColor(0, 0, 0, 128);
// 								ImColor const col_border_skip = ImColor(0, 0, 0, 128);
 								bool const clkd = IconButton(ctx, icoid/*, skip_taskman ? col_int_skip_taskman : col_int, skip_taskman ? col_border_skip : col_border, framing*/);
// 
// 								if (ImGui::IsMouseDragging() && ImGui::IsItemActive())
// 								{
// 									if (!m_dragged)
// 									{
// 										m_dragged = t.m_hwnd;
// 									}
// 									void * texid = nullptr;
// 									float u0 = 0.0f;
// 									float v0 = 0.0f;
// 									float u1 = 0.0f;
// 									float v1 = 0.0f;
// 									/// @TODO: @NOTE: PERFORMANCE !!!
// 									if (bb::BlackBox::Instance().GetGfx().FindIconCoords(icoid, texid, u0, v0, u1, v1))
// 									{
// 										ImDrawList* draw_list = ImGui::GetWindowDrawList();
// 										draw_list->PushClipRectFullScreen();
// 										ImVec2 const mp = ImGui::GetIO().MousePos;
// 										ImVec2 const p1(mp.x - icoid.m_size / 2, mp.y - icoid.m_size / 2);
// 										ImVec2 const p2(mp.x + icoid.m_size / 2, mp.y + icoid.m_size / 2);
// 										ImVec2 const rp1(p1.x - 1, p1.y - 1);
// 										ImVec2 const rp2(p2.x - 1, p2.y - 1);
// 										draw_list->AddImage(texid, p1, p2, ImVec2(u0, v0), ImVec2(u1, v1), col_border);
// 										draw_list->AddRect(rp1, rp2, col_border);
// 										draw_list->PopClipRect();
// 									}
// 								}
// 
// 								if (ImGui::BeginPopupContextItem(""))
// 								{
// 									if (ImGui::Selectable(is_sticky ? "UnStick" : "Stick"))
// 									{
// 										if (is_sticky)
// 											tasks.UnsetSticky(t.m_hwnd);
// 										else
// 											tasks.SetSticky(t.m_hwnd);
// 									}
// 
// 									if (ImGui::Selectable(skip_taskman ? "back to TaskMan" : "rm from TaskMan"))
// 									{
// 										if (skip_taskman)
// 											tasks.UnsetTaskManIgnored(t.m_hwnd);
// 										else
// 											tasks.SetTaskManIgnored(t.m_hwnd);
// 									}
// 
// 									if (ImGui::Button("Close Menu"))
// 										ImGui::CloseCurrentPopup();
// 									ImGui::EndPopup();
// 								}
								if (clkd)
								{
									tasks.Focus(t.m_hwnd);
								}
// 								ImGui::PopID();
 							}
						}
					}
					//ImGui::NextColumn();
				}
				//ImGui::Separator();
			}
		}
// 		ImGuiWindow * w = ImGui::GetCurrentWindowRead();
// 		ImVec2 const & sz1 = w->SizeContents;
// 
// 		ImVec2 const  pad = w->WindowPadding;
// 		ImGuiStyle & style = ImGui::GetStyle();
// 		ImVec2 grr = style.DisplaySafeAreaPadding;
// 		ImVec2 const spac = style.ItemSpacing;
// 		
// 		//window->ScrollbarY = ((window->SizeContents.y > );
// 		//window->ScrollbarX = ((window->SizeContents.x > window->Size.x - (window->ScrollbarY ? style.ScrollbarSize : 0.0f) - window->WindowPadding.x) ;
// 
// 		float sz = w->ScrollbarSizes.x;
// 		float szb = w->ScrollbarY ? style.ScrollbarSize : 0.0f;
// 		float yy = w->SizeContents.y + style.ItemSpacing.y;
// 		float xx = w->SizeContents.x + sz + w->WindowPadding.x;
// 
// 		//ImVec2 const sz2(sz1.x + w->ScrollbarSizes.x + pad.x, sz1.y + w->ScrollbarSizes.y + pad.y/* + spac.y*/);
// 		//ImVec2 const & sz1 = w->SizeFull;
// 		m_contentSize = ImVec2(xx, yy);
// 		ImGui::End();
			nk_end(ctx);
 	}

}}

