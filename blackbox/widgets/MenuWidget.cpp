#include "MenuWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
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
	}

	void MenuWidget::DrawUI ()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);

		ImVec2 const & display = ImGui::GetIO().DisplaySize;

		if (m_contentSize.x > 0 && m_contentSize.y > 0)
		{
			ImGuiStyle & style = ImGui::GetStyle();
			resizeWindowToContents(m_hwnd, m_contentSize.x, m_contentSize.y, style.WindowMinSize.x, style.WindowMinSize.y, style.WindowRounding);
		}

		char name[256];
		codecvt_utf16_utf8(GetNameW(), name, 256);
		ImGui::Begin(name, &m_config.m_show, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::End();

		ImGuiWindow * w = ImGui::GetCurrentWindowRead();
		ImVec2 const & sz1 = w->SizeContents;
		m_contentSize = sz1;
	}

}
