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
		codecvt_utf16_utf8(m_config.m_displayName.c_str(), name, 256);
		bool win_opened = true;
		if (!ImGui::Begin(name, &win_opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::End();
			return;
		}

		if (!win_opened)
		{
			ImGui::End();
			m_gfxWindow->SetDestroyTree();
			return;
		}

		size_t const sz = m_config.m_items.size();
		for (size_t i = 0; i < sz; ++i)
		{
			std::shared_ptr<MenuConfigItem> item = m_config.m_items[i];
			MenuItemDrawUI(i, item);
		}

		ImGuiWindow * w = ImGui::GetCurrentWindowRead();
		ImVec2 const & sz1 = w->SizeContents;
		m_contentSize = sz1;

		ImGui::End();
	}

	void MenuWidget::MenuItemDrawUI (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		switch (item->m_type)
		{
			case e_MenuItemSeparator: DrawSeparator(idx, item); return;
			case e_MenuItemSubMenu: DrawSubMenu(idx, item); return;
			case e_MenuItemScript: DrawScript(idx, item); return;
			case e_MenuItemInt: DrawInt(idx, item); return;
			case e_MenuItemCheckBox: DrawCheckBox(idx, item); return;
			case e_MenuItemBroam: DrawBroam(idx, item); return;
			case e_MenuItemBroamBool: DrawBroamBool(idx, item); return;
			case e_MenuItemBroamInt: DrawBroamInt(idx, item); return;
			case e_MenuItemBroamString: DrawBroamString(idx, item); return;
// 		e_MenuItemFolder,
// 		e_MenuItemExec,
			default:
				Assert(0);
				break;
		}
	}
	void MenuWidget::DrawBroam (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		const bool item_selected = (idx == m_currentIndex);
		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);

		ImGui::PushID(idx);
		if (ImGui::Selectable(item_text, item_selected))
		{
			MenuConfigItemBroam const * broam = static_cast<MenuConfigItemBroam const *>(item.get());

			bb::BlackBox::Instance().GetBroamServer().PostCommand(broam->m_broam.c_str());

			m_gfxWindow->GetRoot()->SetDestroyTree();

			m_currentIndex = idx;
		}
		ImGui::PopID();
	}
	void MenuWidget::DrawBroamInt (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		Assert(item->m_type == e_MenuItemBroamInt);
		MenuConfigItemBroamInt * ibroam = static_cast<MenuConfigItemBroamInt *>(item.get());

		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);
		if (ImGui::SliderInt(item_text, &ibroam->m_val, ibroam->m_min, ibroam->m_max))
		{
			//bb::BlackBox::Instance().GetBroamServer().PostCommand(ibroam->m_broam.c_str());
			if (ibroam->m_broam.find(L"%d") != std::string::npos)
				bb::BlackBox::Instance().GetBroamServer().PostCommand(ibroam->m_broam.c_str(), ibroam->m_val);
			else
				bb::BlackBox::Instance().GetBroamServer().PostCommand(L"%s %d", ibroam->m_broam.c_str(), ibroam->m_val);
		}
	}
	void MenuWidget::DrawBroamBool (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		Assert(item->m_type == e_MenuItemBroamBool);
		MenuConfigItemBroamBool * bbroam = static_cast<MenuConfigItemBroamBool *>(item.get());

		const bool item_selected = (idx == m_currentIndex);
		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);
		if (ImGui::Checkbox(item_text, &bbroam->m_checked))
		{
			bb::BlackBox::Instance().GetBroamServer().PostCommand(L"%s %s", bbroam->m_broam.c_str(), bbroam->m_checked ? L"true" : L"false");

			m_gfxWindow->GetRoot()->SetDestroyTree();
		}
	}
	void MenuWidget::DrawBroamString (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		Assert(item->m_type == e_MenuItemBroamString);
		MenuConfigItemBroamString * sbroam = static_cast<MenuConfigItemBroamString *>(item.get());

		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);
		char inputBuf[256] = {0};
		codecvt_utf16_utf8(sbroam->m_text.c_str(), inputBuf, 256);
		if (ImGui::InputText(item_text, inputBuf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			codecvt_utf8_utf16(inputBuf, sbroam->m_text);

			if (sbroam->m_broam.find(L"%s") != std::string::npos)
				bb::BlackBox::Instance().GetBroamServer().PostCommand(sbroam->m_broam.c_str(), sbroam->m_text.c_str());
			else
				bb::BlackBox::Instance().GetBroamServer().PostCommand(L"%s %s", sbroam->m_broam.c_str(), sbroam->m_text.c_str());
		}
	}

	void MenuWidget::DrawCheckBox (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		Assert(item->m_type == e_MenuItemCheckBox);

		MenuConfigItemCheckBox const * chk_item = static_cast<MenuConfigItemCheckBox const *>(item.get());
		char item_val[1024];
		codecvt_utf16_utf8(chk_item->m_getScript.c_str(), item_val, 1024);
		char response[4096];
		bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

		// @TODO: parse response... is there a better way to do this?
		bool chk = strstr(response, "#t") != 0;

		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);

		bool state = chk;
		if (ImGui::Checkbox(item_text, &state))
		{
			if (state)
			{
				char item_val[1024];
				codecvt_utf16_utf8(chk_item->m_onCheckScript.c_str(), item_val, 1024);
				char response[4096];
				bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

				m_gfxWindow->GetRoot()->SetDestroyTree();
			}
			else
			{
				char item_val[1024];
				codecvt_utf16_utf8(chk_item->m_onUncheckScript.c_str(), item_val, 1024);
				char response[4096];
				bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

				m_gfxWindow->GetRoot()->SetDestroyTree();
			}
		}
	}

	void MenuWidget::DrawInt (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		MenuConfigItemInt * intitem = static_cast<MenuConfigItemInt *>(item.get());

		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);
		ImGui::SliderInt(item_text, &intitem->m_val, intitem->m_min, intitem->m_max);
	}
	void MenuWidget::DrawScript (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		const bool item_selected = (idx == m_currentIndex);
		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);

		ImGui::PushID(idx);
		if (ImGui::Selectable(item_text, item_selected))
		{
			MenuConfigItemScript const * script = static_cast<MenuConfigItemScript const *>(item.get());
			char item_val[1024];
			codecvt_utf16_utf8(script->m_script.c_str(), item_val, 1024);
			char response[4096];
			bb::BlackBox::Instance().GetScheme().Eval(item_val, response, 4096);

			m_gfxWindow->GetRoot()->SetDestroyTree();

			m_currentIndex = idx;
		}
		ImGui::PopID();
	}

	void MenuWidget::DrawSeparator (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		ImGui::Separator();
	}

	void MenuWidget::DrawSubMenu (size_t idx, std::shared_ptr<MenuConfigItem> item)
	{
		const bool item_selected = (idx == m_currentIndex);
		char item_text[1024];
		codecvt_utf16_utf8(item->m_name.c_str(), item_text, 1024);

		ImGui::Bullet();

		ImGui::PushID(idx);
		if (ImGui::Selectable(item_text, item_selected))
		{
			// close other
			for (bb::GfxWindow * w : m_gfxWindow->m_children)
			{
				w->SetDestroyTree();
			}
			m_gfxWindow->m_children.clear();

			ImGui::SameLine();
			ImGui::Bullet();
			// pos of submenu
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->PushClipRectFullScreen();
			ImVec2 a = ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f);
			ImVec2 b = ImGui::GetContentRegionMax();
			draw_list->PopClipRect();

			MenuConfigItemSubMenu const * submenu = static_cast<MenuConfigItemSubMenu const *>(item.get());
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
				int const hdr_size = 24; // @TODO 
				w->MoveWindow(r.left + r.right - r.left, r.top + a.y - hdr_size);
			}

			m_currentIndex = idx;
		}
		ImGui::PopID();
	}

}}
