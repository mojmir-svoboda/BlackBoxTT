#pragma once
#include <platform_win.h>
#include <WinUser.h>

namespace bb {

	inline bool getIconBmpDesc (HICON ico, ICONINFO & info, BITMAP & b)
	{
		memset(&info, 0, sizeof(ICONINFO));
		memset(&b, 0, sizeof(BITMAP));
		if (::GetIconInfo(ico, &info))
			if (0 != ::GetObject(info.hbmColor, sizeof(b), &b))
				return true;
		return false;
	}

	inline size_t iconToBuffer (ICONINFO & iconInfo, BITMAP & bmpInfo, uint8_t * buff, size_t buffsz)
	{
		size_t const sz = bmpInfo.bmWidth * bmpInfo.bmHeight * bmpInfo.bmBitsPixel / 8;
		memset(buff, 0, sz);

		BITMAPINFO lBitmapInfo;
		memset(&lBitmapInfo, 0, sizeof(BITMAPINFO));
		lBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		lBitmapInfo.bmiHeader.biWidth = bmpInfo.bmWidth;
		lBitmapInfo.bmiHeader.biHeight = bmpInfo.bmHeight * -1;
		lBitmapInfo.bmiHeader.biPlanes = 1;
		lBitmapInfo.bmiHeader.biBitCount = bmpInfo.bmBitsPixel;

		HDC hdc = ::GetDC(nullptr);
		if (0 != ::GetDIBits(hdc, iconInfo.hbmColor, 0, bmpInfo.bmHeight, buff, &lBitmapInfo, DIB_RGB_COLORS))
		{
			::DeleteDC(hdc);
			return sz;
		}
		::DeleteDC(hdc);
		return 0;
	}

	inline size_t iconToBuffer (HICON ico, uint8_t * buff, size_t buffsz, int & x, int & y, int & bits)
	{
		ICONINFO iconInfo;
		BITMAP b;
		if (getIconBmpDesc(ico, iconInfo, b))
		{
			if (size_t const sz = iconToBuffer(iconInfo, b, buff, buffsz))
			{
				x = b.bmWidth;
				y = b.bmHeight;
				bits = b.bmBitsPixel;
				return sz;
			}
		}
		return 0;
	}

	inline HICON getTaskIconSmall (HWND hwnd)
	{
		if (HICON hico = reinterpret_cast<HICON>(::GetClassLongPtr(hwnd, GCLP_HICONSM))) // GCL_ on win32?
			return hico;
		if (HICON hico = reinterpret_cast<HICON>(::SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0)))
			return hico;
		if (HICON hico = reinterpret_cast<HICON>(::SendMessage(hwnd, WM_GETICON, ICON_SMALL2, 0)))
			return hico;
		return nullptr;
	}

	inline HICON getTaskIconLarge (HWND hwnd)
	{
		if (HICON hico = reinterpret_cast<HICON>(::GetClassLongPtr(hwnd, GCLP_HICON))) // GCL_ on win32?
			return hico;
		if (HICON hico = reinterpret_cast<HICON>(::SendMessage(hwnd, WM_GETICON, ICON_BIG, 0)))
			return hico;
		return nullptr;
	}

}
