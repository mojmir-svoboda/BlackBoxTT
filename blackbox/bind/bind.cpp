#include "bind.h"
#include <BlackBox.h>
#include <bblib/codecvt.h>

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

	BB_API void SetCurrentVertexId (char const * vertex)
	{
		bb::BlackBox * const bb = getBlackBoxInstanceRW();

		size_t const ln = strlen(vertex);
		size_t const sz = bb::codecvt_utf8_utf16_dst_size(vertex, ln);
		wchar_t * const bbcmd_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
		size_t const bbcmd_u16_ln = bb::codecvt_utf8_utf16(vertex, ln, bbcmd_u16, sz);
		bbstring b(bbcmd_u16, bbcmd_u16_ln);

		bb->GetWorkSpaces().SetCurrentVertexId(b);
	}
}

