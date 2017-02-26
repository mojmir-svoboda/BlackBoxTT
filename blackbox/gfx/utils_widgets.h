#pragma once

namespace bb {

	void moveWidgetToPointerPos (bb::GuiWidget * w)
	{
		POINT p;
		if (::GetCursorPos(&p))
		{
			w->MoveWindow(p.x, p.y);
		}
	}
}


