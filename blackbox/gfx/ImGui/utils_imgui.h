#pragma once
#include "Gui.h"
#include <imgui/imgui.h>
#include "DX11.h"
#include "BlackBox.h"

namespace ImGui
{
	inline bool Icon (bb::IconId id, const ImVec4 & tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0))
	{
		void * texid = nullptr;
		float u0 = 0.0f;
		float v0 = 0.0f;
		float u1 = 0.0f;
		float v1 = 0.0f;
		if (bb::BlackBox::Instance().GetGfx().FindIconCoords(id, texid, u0, v0, u1, v1))
		{
			ImGui::Image(texid, ImVec2(id.m_size, id.m_size), ImVec2(u0, v0), ImVec2(u1, v1), tint_col, border_col);
			return true;
		}
		if (id.IsValid())
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX, "Error - cannot find icon, iconid=(sz=%i, slb=%i, idx=%i)", id.m_size, id.m_slab, id.m_index);
		return false;
	}
	inline bool IconButton (bb::IconId id, const ImVec4 & tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0), int framing = -1)
	{
		void * texid = nullptr;
		float u0 = 0.0f;
		float v0 = 0.0f;
		float u1 = 0.0f;
		float v1 = 0.0f;
		if (bb::BlackBox::Instance().GetGfx().FindIconCoords(id, texid, u0, v0, u1, v1))
		{
			return ImGui::ImageButton(texid, ImVec2(id.m_size, id.m_size), ImVec2(u0, v0), ImVec2(u1, v1), framing, tint_col, border_col);
		}
		if (id.IsValid())
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX, "Error - cannot find icon, iconid=(sz=%i, slb=%i, idx=%i)", id.m_size, id.m_slab, id.m_index);
		return false;
	}

	// Play it nice with Windows users. Notepad in 2015 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif
	inline void ShowStyleEditor(ImGuiStyle* ref)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		const ImGuiStyle def; // Default style
		if (ImGui::Button("Revert Style"))
			style = ref ? *ref : def;
		if (ref)
		{
			ImGui::SameLine();
			if (ImGui::Button("Save Style"))
				*ref = style;
		}

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.55f);

		if (ImGui::TreeNode("Rendering"))
		{
			ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
			ImGui::Checkbox("Anti-aliased shapes", &style.AntiAliasedShapes);
			ImGui::PushItemWidth(100);
			ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, FLT_MAX, NULL, 2.0f);
			if (style.CurveTessellationTol < 0.0f) style.CurveTessellationTol = 0.10f;
			ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
			ImGui::PopItemWidth();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Sizes"))
		{
			ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 16.0f, "%.0f");
			ImGui::SliderFloat("ChildWindowRounding", &style.ChildWindowRounding, 0.0f, 16.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 16.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 16.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 16.0f, "%.0f");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Colors"))
		{
			static int output_dest = 0;
			static bool output_only_modified = false;
			if (ImGui::Button("Copy Colors"))
			{
				if (output_dest == 0)
					ImGui::LogToClipboard();
				else
					ImGui::LogToTTY();
				ImGui::LogText("ImGuiStyle& style = ImGui::GetStyle();" IM_NEWLINE);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const ImVec4& col = style.Colors[i];
					const char* name = ImGui::GetStyleColName(i);
					if (!output_only_modified || memcmp(&col, (ref ? &ref->Colors[i] : &def.Colors[i]), sizeof(ImVec4)) != 0)
						ImGui::LogText("style.Colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 22 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
				}
				ImGui::LogFinish();
			}
			ImGui::SameLine(); ImGui::PushItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY"); ImGui::PopItemWidth();
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Fields", &output_only_modified);

			static ImGuiColorEditMode edit_mode = ImGuiColorEditMode_RGB;
			ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditMode_RGB);
			ImGui::SameLine();
			ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditMode_HSV);
			ImGui::SameLine();
			ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditMode_HEX);
			//ImGui::Text("Tip: Click on colored square to change edit mode.");

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", 200);

			ImGui::BeginChild("#colors", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			ImGui::PushItemWidth(-160);
			ImGui::ColorEditMode(edit_mode);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4(name, (float*)&style.Colors[i], true);
				if (memcmp(&style.Colors[i], (ref ? &ref->Colors[i] : &def.Colors[i]), sizeof(ImVec4)) != 0)
				{
					ImGui::SameLine(); if (ImGui::Button("Revert")) style.Colors[i] = ref ? ref->Colors[i] : def.Colors[i];
					if (ref) { ImGui::SameLine(); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i]; }
				}
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Fonts", "Fonts (%d)", ImGui::GetIO().Fonts->Fonts.Size))
		{
			ImGui::SameLine(); //ShowHelpMarker("Tip: Load fonts with io.Fonts->AddFontFromFileTTF()\nbefore calling io.Fonts->GetTex* functions.");
			ImFontAtlas* atlas = ImGui::GetIO().Fonts;
			if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
			{
				ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				ImGui::TreePop();
			}
			ImGui::PushItemWidth(100);
			for (int i = 0; i < atlas->Fonts.Size; i++)
			{
				ImFont* font = atlas->Fonts[i];
				ImGui::BulletText("Font %d: \'%s\', %.2f px, %d glyphs", i, font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size);
				ImGui::TreePush((void*)(intptr_t)i);
				if (i > 0) { ImGui::SameLine(); if (ImGui::SmallButton("Set as default")) { atlas->Fonts[i] = atlas->Fonts[0]; atlas->Fonts[0] = font; } }
				ImGui::PushFont(font);
				ImGui::Text("The quick brown fox jumps over the lazy dog");
				ImGui::PopFont();
				if (ImGui::TreeNode("Details"))
				{
					ImGui::DragFloat("font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");             // scale only this font
					ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
					ImGui::Text("Fallback character: '%c' (%d)", font->FallbackChar, font->FallbackChar);
					for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
					{
						ImFontConfig* cfg = &font->ConfigData[config_i];
						ImGui::BulletText("Input %d: \'%s\'\nOversample: (%d,%d), PixelSnapH: %d", config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH);
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			static float window_scale = 1.0f;
			ImGui::DragFloat("this window scale", &window_scale, 0.005f, 0.3f, 2.0f, "%.1f");              // scale only this window
			ImGui::DragFloat("global scale", &ImGui::GetIO().FontGlobalScale, 0.005f, 0.3f, 2.0f, "%.1f"); // scale everything
			ImGui::PopItemWidth();
			ImGui::SetWindowFontScale(window_scale);
			ImGui::TreePop();
		}

		ImGui::PopItemWidth();
	}
#undef IM_NEWLINE

}

