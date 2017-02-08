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
				MenuConfigItem const & item = m_config.m_items[i];
				const bool item_selected = (i == m_currentIndex);
				char item_text[1024];
				codecvt_utf16_utf8(item.m_name.c_str(), item_text, 1024);

				//ImGui::SameLine();
				ImGui::PushID(i);
				//ImGui::SameLine();
				if (ImGui::Selectable(item_text, item_selected))
				{
					// BB::CreateMenu()
					if (item.m_type == e_MenuItemMenu)
					{	
						//MenuWidget * w = bb::BlackBox::Instance().CreateMenuOnPointerPos(*item.m_menu);

						MenuConfig const * const cfg = item.m_menu.get();
						GuiWidget * w = bb::BlackBox::Instance().GetGfx().MkWidgetFromConfig(*cfg);

					}

					m_currentIndex = i;
					//value_changed = true;
				}
				ImGui::PopID();
			}
		}

		ImGuiWindow * w = ImGui::GetCurrentWindowRead();
		ImVec2 const & sz1 = w->SizeContents;
		m_contentSize = sz1;

		ImGui::End();
	}

}}
