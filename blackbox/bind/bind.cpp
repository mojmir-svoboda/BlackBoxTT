#include "bind.h"
#include <BlackBox.h>

extern "C" {

	BB_API void bbQuit (uint32_t arg)
	{
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->Quit(arg);
	}

	BB_API void MakeSticky (HWND hwnd)
	{
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->MakeSticky(hwnd);
	}

	BB_API void RemoveSticky (HWND hwnd)
	{
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		bb->RemoveSticky(hwnd);
	}

	BB_API HWND GetBBWnd ()
	{
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		return bb->GetHwnd();
	}

	BB_API void * GetSettingPtr (int sn_index)
	{
		bb::BlackBox * const bb = getBlackBoxInstanceRW();
		return bb->GetSettingPtr(sn_index);
	}

	BB_API bool GetConfigDir (wchar_t * dir, size_t dir_sz)
	{
		bb::BlackBox const * const bb = getBlackBoxInstance();
		return bb->GetConfigDir(dir, dir_sz);
	}

}

