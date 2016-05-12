#include "foompbutton.h"
#include "styles.h"
#include "settings.h"

void FoompButton::draw (HDC buf)
{
	//Create drawing tools
	HPEN hDefPen = CreatePen(PS_SOLID, 1, getStyles().OuterStyle.TextColor);
	HPEN hPressedPen = CreatePen(PS_SOLID, 1, getStyles().ButtonStyle.TextColor);
	HPEN hShadowPen = CreatePen(PS_SOLID, 1, GetShadowColor(getStyles().OuterStyle));
	HPEN hPressedShadowPen = CreatePen(PS_SOLID, 1, GetShadowColor(getStyles().ButtonStyle));
	//Save current object
	HGDIOBJ prev = SelectObject(buf,hDefPen);

	if (pressed)
		MakeStyleGradient(buf, &hitrect, &getStyles().ButtonStyle, false);

	if (getSettings().FooShadowsEnabled)
	{
		SelectObject(buf,pressed ? hPressedShadowPen : hShadowPen);
		drawShape(buf,x+1,y+1);
	}
	SelectObject(buf,pressed ? hPressedPen : hDefPen);
	drawShape(buf,x,y);


	//Revert old object
	SelectObject(buf, prev);
	//Destroy drawing tools
	DeleteObject(hDefPen);
	DeleteObject(hPressedPen);
	DeleteObject(hShadowPen);
	DeleteObject(hPressedShadowPen);
}

void FoompButton::drawShape (HDC buf, int Penx, int Peny)
{
	switch (type)
	{
		case BUTTON_REWIND:
			//-------		First arrow
			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny);
			MoveToEx(buf, Penx+1, Peny+4, NULL);
			LineTo(buf, Penx+1, Peny+1);
			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx, Peny+2);
			
			//-------		2nd arrow
			MoveToEx(buf, Penx+7, Peny+5, NULL);
			LineTo(buf, Penx+7, Peny);
			MoveToEx(buf, Penx+6, Peny+4, NULL);
			LineTo(buf, Penx+6, Peny+1);
			MoveToEx(buf, Penx+5, Peny+3, NULL);
			LineTo(buf, Penx+5, Peny+2);

			break;
		case BUTTON_PLAY:
			//=======	Begin Draw Play
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+4, NULL);
			LineTo(buf, Penx+1, Peny+1);
			MoveToEx(buf, Penx+2, Peny+3, NULL);
			LineTo(buf, Penx+2, Peny+2);
			
			//=======	End Draw Play
			break;
		case BUTTON_PAUSE:
			//=======	Begin Draw Pause
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+5, NULL);
			LineTo(buf, Penx+1, Peny);
			//MoveToEx(buf, Penx+2, Peny+5, NULL);
			//LineTo(buf, Penx+2, Peny);
			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny);
			MoveToEx(buf, Penx+4, Peny+5, NULL);
			LineTo(buf, Penx+4, Peny);

			//=======	End Draw Pause
			break;
		case BUTTON_STOP:
			//=======	Begin Draw Stop
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+5, NULL);
			LineTo(buf, Penx+1, Peny);
			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny);
			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny);
			MoveToEx(buf, Penx+4, Peny+5, NULL);
			LineTo(buf, Penx+4, Peny);

				// *** ATTN: No need for "rStop" because rPause == "rStop" == "rPls"
			//=======	End Draw Stop
			break;
		case BUTTON_FORWARD:
			
			//=======	Begin Draw Forward
			//-------		First arrow
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+4, NULL);
			LineTo(buf, Penx+1, Peny+1);
			MoveToEx(buf, Penx+2, Peny+3, NULL);
			LineTo(buf, Penx+2, Peny+2);

			//-------		2nd arrow
			MoveToEx(buf, Penx+5, Peny+5, NULL);
			LineTo(buf, Penx+5, Peny);
			MoveToEx(buf, Penx+6, Peny+4, NULL);
			LineTo(buf, Penx+6, Peny+1);
			MoveToEx(buf, Penx+7, Peny+3, NULL);
			LineTo(buf, Penx+7, Peny+2);
			
				// *** ATTN: No need for "rFwd" because rRew == "rFwd"
			//=======	End Draw Forward
			break;
		case BUTTON_PLAYLIST:
			//=======	Begin Draw Playlist
			MoveToEx(buf, Penx, Peny+1, NULL);
			LineTo(buf, Penx+5, Peny+1);
			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+5, Peny+3);
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx+5, Peny+5);

				// *** ATTN: No need for "rStop" because rPause == "rStop" == "rPls"
			//=======	End Draw Playlist
			break;
		case BUTTON_OPEN:
			//=======	Begin Draw Addfiles
			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+5, Peny+3);

			MoveToEx(buf, Penx+1, Peny+2, NULL);
			LineTo(buf, Penx+4, Peny+2);

			MoveToEx(buf, Penx+2, Peny+1, NULL);
			LineTo(buf, Penx+3, Peny+1);

			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx+5, Peny+5);

			//=======	End Draw Add files		
			break;
		case BUTTON_UPARROW:
			//=======	Begin Draw Volume Up
			MoveToEx(buf, Penx+5, Peny+3, NULL);
			LineTo(buf, Penx+1, Peny);

			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+3, Peny);

			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny+1);

			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny+1);

			//=======	End Draw Volume Up		
			break;
		case BUTTON_DOWNARROW:
			//=======	Begin Draw Volume Down
			MoveToEx(buf, Penx+5, Peny+3, NULL);
			LineTo(buf, Penx+1, Peny+6);

			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+3, Peny+6);

			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny);

			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny);

			//=======	End Draw Volume Down
			break;
	}
}

int FoompButton::width ()
{
	switch (type)
	{
		case BUTTON_REWIND:		return 8;
		case BUTTON_PLAY:		return 3;
		case BUTTON_PAUSE:		return 5;
		case BUTTON_STOP:		return 5;
		case BUTTON_FORWARD:	return 8;
		case BUTTON_PLAYLIST:	return 5;
		case BUTTON_OPEN:		return 5;
		case BUTTON_UPARROW:	return 6;
		case BUTTON_DOWNARROW:	return 6;
		default:				return 0;
	}
}

int FoompButton::height ()
{
	return 5;
}

bool FoompButton::clicked (int mouseX, int mouseY)
{
	return	(	(mouseY >= hitrect.top)
			&&	(mouseY <= hitrect.bottom)
			&&	(mouseX >= hitrect.left)
			&&	(mouseX <= hitrect.right)
			);
}

