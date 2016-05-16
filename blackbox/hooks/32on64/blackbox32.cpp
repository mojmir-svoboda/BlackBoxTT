#include <platform_win.h>
#include <hooks/shellhook.h>

// https://blogs.msdn.microsoft.com/oldnewthing/20031211-00/?p=41543
#include <Shlwapi.h>
void initShellHook32 (LPSTR pszCmdLine)
{
	LONGLONG llHandle;
	if (StrToInt64ExA(pszCmdLine, STIF_DEFAULT, &llHandle))
	{
		HWND bbHwnd = (HWND)(INT_PTR)llHandle;
		initShellHook32(bbHwnd);
	}
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pszCmdLine, int iCmdShow)
{
	//system("pause");
	initShellHook32(pszCmdLine);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) && WM_QUIT != msg.message)
	{
  	TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  doneShellHook();
  return static_cast<int>(msg.wParam);
}

