#include "MenuWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <blackbox/utils_window.h>
#include <bblib/codecvt.h>
#include <imgui/imgui_internal.h>
#include <yaml-cpp/yaml.h>
#include "utils_yaml.h"
#include "WidgetConfig_yaml.h"
#include <blackbox/menu/MenuConfig_yaml.h>

namespace bb {
namespace imgui {

	MenuWidget::MenuWidget ()
	{
	}

	MenuWidget::~MenuWidget ()
	{
	}

	bool MenuWidget::loadConfig (YAML::Node & y_cfg_node)
	{
		if (!y_cfg_node.IsNull())
		{
			MenuConfig tmp = y_cfg_node.as<MenuConfig>();
			m_config = std::move(tmp);
			return true;
		}
		return false;
	}

	void MenuWidget::DrawUI ()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;

		if (m_contentSize.x > 0 && m_contentSize.y > 0)
		{
			ImGuiStyle & style = ImGui::GetStyle();
			resizeWindowToContents(m_gfxWindow->m_hwnd, m_contentSize.x, m_contentSize.y, style.WindowMinSize.x, style.WindowMinSize.y, style.WindowRounding);
		}

		char name[256];
		codecvt_utf16_utf8(GetId(), name, 256);
		bool close_menu = false;
		if (!ImGui::Begin(name, &close_menu, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::End();
			m_gfxWindow->SetDestroy(true);
			return;
		}

		size_t const sz = m_config.m_items.size();

		{
			//bool value_changed = false;
			for (size_t i = 0; i < sz; ++i)
			{
				MenuConfigItem const * item = m_config.m_items[i].get();
				const bool item_selected = (i == m_currentIndex);
				char item_text[1024];
				codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);

				if (item->m_type == e_MenuItemSeparator)
				{
					ImGui::Separator();
					continue;
				}

				if (item->m_type == e_MenuItemCheckBox)
				{

					MenuConfigItemCheckBox const * chk_item = static_cast<MenuConfigItemCheckBox const *>(item);
					char item_val[1024];
					codecvt_utf16_utf8(chk_item->m_getScript.c_str(), item_val, 1024);
					char response[4096];
					bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

					// @TODO: parse response!!!

					bool state = false;
					if (ImGui::Checkbox(item_text, &state))
					{
						if (state)
						{
							char item_val[1024];
							codecvt_utf16_utf8(chk_item->m_onCheckScript.c_str(), item_val, 1024);
							char response[4096];
							bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

							bb::GfxWindow * r = m_gfxWindow->GetRoot();
							r->SetDestroyTree();

						}
						else
						{
							char item_val[1024];
							codecvt_utf16_utf8(chk_item->m_onUncheckScript.c_str(), item_val, 1024);
							char response[4096];
							bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

							bb::GfxWindow * r = m_gfxWindow->GetRoot();
							r->SetDestroyTree();
						}

					}
				}
				else
				{
					//ImGui::SameLine();
					ImGui::PushID(i);
					//ImGui::SameLine();
					if (ImGui::Selectable(item_text, item_selected))
					{
						// close other
						for (bb::GfxWindow * w : m_gfxWindow->m_children)
						{
							w->SetDestroyTree();
						}
						m_gfxWindow->m_children.clear();

						if (item->m_type == e_MenuItemScript)
						{
							MenuConfigItemScript const * script = static_cast<MenuConfigItemScript const *>(item);
							char item_val[1024];
							codecvt_utf16_utf8(script->m_script.c_str(), item_val, 1024);
							char response[4096];
							bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

							bb::GfxWindow * r = m_gfxWindow->GetRoot();
							r->SetDestroyTree();
						}
						else if (item->m_type == e_MenuItemBroam)
						{
							MenuConfigItemBroam const * script = static_cast<MenuConfigItemBroam const *>(item);
							char item_val[1024];
							codecvt_utf16_utf8(script->m_broam.c_str(), item_val, 1024);
							char response[4096];
							//bb::BlackBox::Instance().m_broamServer.HandleBroam()

							bb::GfxWindow * r = m_gfxWindow->GetRoot();
							r->SetDestroyTree();
						}
						else if (item->m_type == e_MenuItemSubMenu)
						{	
							// pos of submenu
							ImDrawList* draw_list = ImGui::GetWindowDrawList();
							draw_list->PushClipRectFullScreen();
							ImVec2 a = ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f);
							ImVec2 b = ImGui::GetContentRegionMax();
 							draw_list->PopClipRect();

							MenuConfigItemSubMenu const * submenu = static_cast<MenuConfigItemSubMenu const *>(item);
							// open menu
							GuiWidget * w = bb::BlackBox::Instance().GetGfx().FindWidget(submenu->m_menu->m_id.c_str());
							if (w == nullptr)
							{
								w = bb::BlackBox::Instance().GetGfx().MkWidgetFromConfig(*submenu->m_menu);
								m_gfxWindow->AddChild(w->m_gfxWindow);
								w->m_gfxWindow->SetParent(m_gfxWindow);
							}
							RECT r;
							::GetWindowRect(m_gfxWindow->m_hwnd, &r);
							{
								int const hdr_size = 24;
								w->MoveWindow(r.left + r.right - r.left, r.top + a.y - hdr_size);
							}
						}

						m_currentIndex = i;
						//value_changed = true;
					}
					ImGui::PopID();
				}
			}
		}

		ImGuiWindow * w = ImGui::GetCurrentWindowRead();
		ImVec2 const & sz1 = w->SizeContents;
		m_contentSize = sz1;

		ImGui::End();
	}

}}
