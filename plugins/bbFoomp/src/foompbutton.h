#pragma once
#include <blackbox/plugin/bb.h>

enum ButtonType
{
	BUTTON_REWIND,
	BUTTON_PLAY,
	BUTTON_PAUSE,
	BUTTON_STOP,
	BUTTON_FORWARD,
	BUTTON_PLAYLIST,
	BUTTON_OPEN,
	BUTTON_UPARROW,
	BUTTON_DOWNARROW,
  e_last_button_item
};

struct FoompButton
{
	RECT hitrect;
	int x, y;
	ButtonType type;
	char cmdarg[256];
	char altcmdarg[256];
	bool pressed;

	void draw (HDC buf);
	int width ();
	int height ();
	bool clicked (int mouseX, int mouseY);

private:
	void drawShape (HDC buf, int Penx, int Peny);
};


