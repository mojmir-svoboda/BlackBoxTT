#pragma once
#include "platform_win.h"
#include <vector>
#include "Gui.h"
#include <blackbox/gfx/Gfx.h>
#include <blackbox/gfx/GuiWidget.h>
#include <blackbox/gfx/GfxWindow.h>
#include <blackbox/gfx/shared/DX11.h>
#include <blackbox/gfx/shared/Gfx.h>
//#include "IconCache.h"
#include <blackbox/WidgetConfig.h>
#include <blackbox/GfxConfig.h>
#include <blackbox/Tasks.h>
namespace YAML { class Node; }

namespace bb {
namespace imgui {

	struct Gfx : bb::shared::Gfx
	{
		int												m_VertexBufferSize { 5000 };
		int												m_IndexBufferSize { 10000 };

		Gfx (Tasks & t, YAML::Node & y_root) : bb::shared::Gfx(t, y_root) { }
		virtual ~Gfx ();
		virtual bool Done () override;
		virtual void ResetInput () override { }
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType) override;
		virtual std::unique_ptr<GuiWidget> MkWidgetFromType (wchar_t const * widgetType, WidgetConfig const & cfg) override;
		virtual GuiWidget * MkWindowForWidget (int x, int y, int w, int h, int a, std::unique_ptr<GuiWidget> && widget) override;
		virtual bool CreateDeviceObjects () override;
		virtual void ReleaseDeviceObjects () override;
		virtual void CreateFontsTexture () override;
		virtual void RenderImGui () override;
	};

}}

