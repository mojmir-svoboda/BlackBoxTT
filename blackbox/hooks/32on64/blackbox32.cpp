#include <platform_win.h>
#include <hooks/taskhook.h>

// https://blogs.msdn.microsoft.com/oldnewthing/20031211-00/?p=41543
#include <Shlwapi.h>
void initTaskHook32 (LPSTR pszCmdLine)
{
	LONGLONG llHandle;
	if (StrToInt64ExA(pszCmdLine, STIF_DEFAULT, &llHandle))
	{
		HWND bbHwnd = (HWND)(INT_PTR)llHandle;
		initTaskHook32(bbHwnd);
	}
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pszCmdLine, int iCmdShow)
{
	//system("pause");
	initTaskHook32(pszCmdLine);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) && WM_QUIT != msg.message)
	{
  	TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  doneTaskHook();
  return static_cast<int>(msg.wParam);
}

