#pragma once
#include "platform_win.h"
#include <vector>
#include "Gui.h"
#include <blackbox/gfx/Gfx.h>
#include <blackbox/gfx/shared/Gfx.h>
#include <blackbox/gfx/shared/GfxWindow.h>
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/WidgetConfig.h>
#include <blackbox/GfxConfig.h>
#include <blackbox/Tasks.h>
namespace YAML { class Node; }

namespace bb {
namespace nuklear {

	struct Gfx : bb::shared::Gfx
	{
		D3D11_VIEWPORT m_viewport { };
		nk_context m_context { };
		nk_buffer m_cmd { };
		nk_font_atlas m_atlas { };

		size_t												m_VertexBufferSize { 512 * 1024 };
		size_t												m_IndexBufferSize { 128 * 1024 };
		nk_draw_null_texture m_nullTexture { };

		Gfx (Tasks & t, YAML::Node & y_root) : bb::shared::Gfx(t, y_root) { }
		virtual ~Gfx ();
		virtual bool Done () override;
		virtual void ResetInput () override;
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType) override;
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg) override;
		virtual GuiWidget * MkWindowForWidget (int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget) override;
		virtual void DeviceLost () override;
		virtual bool CreateDeviceObjects () override;
		virtual void ReleaseDeviceObjects () override;
		virtual void CreateFontsTexture () override;
		virtual void RenderImGui ();
	};

}}

