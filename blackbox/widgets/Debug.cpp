#include "Debug.h"
#include <blackbox/BlackBox.h>
#include <blackbox/gfx/utils_imgui.h>
#include <bblib/codecvt.h>

namespace bb {

	void DebugWidget::DrawUI ()
	{
		ImGui::Text("%.1f ms", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
	}
}
