#include "MenuWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/ImGui/utils_imgui.h>
#include <blackbox/utils_window.h>
#include <bblib/codecvt.h>
#include <imgui/imgui_internal.h>

namespace bb {

	MenuWidget::MenuWidget (WidgetConfig & cfg) 
		: GuiWidget(cfg)
	{
	}

	MenuWidget::~MenuWidget ()
	{
	}

	void MenuWidget::CreateMenuFromConfig (MenuConfig const & cfg)
	{
		m_menuConfig = cfg;
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
		codecvt_utf16_utf8(GetNameW(), name, 256);
		ImGui::Begin(name, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

		size_t const sz = m_menuConfig.m_items.size();

		{
			//bool value_changed = false;
			for (size_t i = 0; i < sz; ++i)
			{
				MenuConfigItem const & item = m_menuConfig.m_items[i];
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
						MenuWidget * w = bb::BlackBox::Instance().CreateMenuOnPointerPos(*item.m_menu);
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

}
