#pragma once
#include "Gui.h"
#include <nuklear/nuklear_config.h>
//#include "DX11.h"
#include "BlackBox.h"

namespace bb {
	inline bool IconButton (nk_context * ctx, bb::IconId id/*, const ImVec4 & tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0), int framing = -1*/)
	{
		void * texid = nullptr;
		uint32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;
		uint32_t szx = 0, szy = 0;
		if (bb::BlackBox::Instance().GetGfx().FindIconCoords(id, texid, szx, szy, x0, y0, x1, y1))
		{
			//return ImGui::ImageButton(texid, ImVec2(id.m_size, id.m_size), ImVec2(u0, v0), ImVec2(u1, v1), framing, tint_col, border_col);
			struct nk_image im = nk_subimage_ptr(texid, szx, szy, struct nk_rect { (float)x0, (float)y0, (float)x1, (float)y1 });
			bool const clkd = nk_button_image(ctx, im);

			return clkd;
		}
		if (!id.IsValid())
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX, "Error - cannot find icon, iconid=(sz=%i, slb=%i, idx=%i)", id.m_size, id.m_slab, id.m_index);
		return false;
	}
}

// using ImVec2 = nk_vec2;
// 
// namespace ImGui
// {
// 	inline bool Icon (bb::IconId id, const ImVec4 & tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0))
// 	{
// 		void * texid = nullptr;
// 		float u0 = 0.0f;
// 		float v0 = 0.0f;
// 		float u1 = 0.0f;
// 		float v1 = 0.0f;
// 		if (bb::BlackBox::Instance().GetGfx().FindIconCoords(id, texid, u0, v0, u1, v1))
// 		{
// 			ImGui::Image(texid, ImVec2(id.m_size, id.m_size), ImVec2(u0, v0), ImVec2(u1, v1), tint_col, border_col);
// 			return true;
// 		}
// 		if (!id.IsValid())
// 			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX, "Error - cannot find icon, iconid=(sz=%i, slb=%i, idx=%i)", id.m_size, id.m_slab, id.m_index);
// 		return false;
// 	}
// 	inline bool IconButton (bb::IconId id, const ImVec4 & tint_col = ImVec4(1, 1, 1, 1), const ImVec4 & border_col = ImVec4(0, 0, 0, 0), int framing = -1)
// 	{
// 		void * texid = nullptr;
// 		float u0 = 0.0f;
// 		float v0 = 0.0f;
// 		float u1 = 0.0f;
// 		float v1 = 0.0f;
// 		if (bb::BlackBox::Instance().GetGfx().FindIconCoords(id, texid, u0, v0, u1, v1))
// 		{
// 			return ImGui::ImageButton(texid, ImVec2(id.m_size, id.m_size), ImVec2(u0, v0), ImVec2(u1, v1), framing, tint_col, border_col);
// 		}
// 		if (!id.IsValid())
// 			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX, "Error - cannot find icon, iconid=(sz=%i, slb=%i, idx=%i)", id.m_size, id.m_slab, id.m_index);
// 		return false;
// 	}
// 
