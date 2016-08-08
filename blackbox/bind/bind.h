#pragma once
#include <cstdint>
#include <platform_win.h>
#include <blackbox/blackbox_api.h>

extern "C" {

	BB_API void bbQuit (uint32_t arg);

	BB_API void MakeSticky (HWND hwnd);// { return getWorkspaces().MakeSticky(hwnd); }
	BB_API void RemoveSticky (HWND hwnd);// { return getWorkspaces().RemoveSticky(hwnd); }
	BB_API HWND GetBBWnd ();

	BB_API /*API_EXPORT */void* GetSettingPtr (int sn_index);

	BB_API bool bbGetConfigDir (wchar_t * dir, size_t dir_sz);

	//BB_API void SetCurrentVertexId (wchar_t const * vertex);
	BB_API void SetCurrentVertexId (char const * vertex);
}

