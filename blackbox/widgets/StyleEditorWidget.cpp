#include "StyleEditorWidget.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void StyleEditorWidget::DrawUI ()
	{
		ImGui::ShowStyleEditor();
	}
}
