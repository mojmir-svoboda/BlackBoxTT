#pragma once
#include "foompbutton.h"
#include <bblib/bbstring.h>

struct Settings
{
  int xpos, ypos;
  int width, height, BorderWidth;
  int FooMode, FooWidth;
  FoompButton buttons[e_last_button_item];
  bool FooDockedToSlit;
  bool FooOnTop, FooTrans, FooAlign, FooShadowsEnabled;
  // Determines style:
  int InnerStyleIndex;
  int OuterStyleIndex;
  int FooScrollSpeed;
  int transparencyAlpha;
  // paths
  bbstring rcpath;
  bbstring FooPath;
  bbstring NoInfoText;

  void ReadRCSettings ();
  void WriteRCSettings ();
  void WriteDefaultRCSettings ();

	Settings ()
		: xpos(0), ypos(0)
		, width(), height(22), BorderWidth(3)
		, FooMode(2), FooWidth(200)
		, FooDockedToSlit(false)
		, FooOnTop(false), FooTrans(false), FooAlign(true), transparencyAlpha(220)
		, InnerStyleIndex(2)
		, OuterStyleIndex(4)
		, FooShadowsEnabled(false)
		, FooScrollSpeed(5)
		, rcpath()
		
#if defined _WIN64
		, FooPath(L"C:\\Program Files (x86)\\foobar2000\\foobar2000.exe") // Foo Directory [FooPath]
#else
		, FooPath(L"C:\\Progra~1\\foobar2000\\foobar2000.exe") // Foo Directory [FooPath]
#endif
		, NoInfoText(L"---")
	{
// 		for (int i = 0; i < e_last_button_item; ++i)
// 		{
// 			FoompButton &b = buttons[i];
// 			char picname[100], cmdname[100], altcmdname[100];
// 			sprintf(picname, "bbfoomp.button%d.image:", i + 1);
// 			sprintf(cmdname, "bbfoomp.button%d.command:", i + 1);
// 			sprintf(altcmdname, "bbfoomp.button%d.altcommand:", i + 1);
// 			b.type = ButtonType(ReadInt(rcpath, picname, i));
// 			_tcscpy(b.cmdarg, ReadString(rcpath, cmdname, default_commands[i]));
// 			_tcscpy(b.altcmdarg, ReadString(rcpath, altcmdname, default_altcommands[i]));
// 		}

		// Minimum settings checks.
		if (height < (15 + BorderWidth) || width < 0 || BorderWidth < 0)
		{
			MessageBox(0, TEXT("The value you have inputted for either: \nheight, width or border-width is below the minimum.\nThe values will default. Please consult the Readme for the minimums."), L"ERROR: Illegal value set.", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
			FooWidth = 200;
			height = 22;
			BorderWidth = 3;
		}
	}
};

Settings & getSettings ();
