#pragma once
#include <d3d11.h>
#include <DXGI1_2.h>
#include <d3dcompiler.h>

namespace bb {

	struct DX11
	{
		ID3D11Device * m_pd3dDevice;
		ID3D11DeviceContext * m_pd3dDeviceContext;
		int const c_sampleCount;

		DX11 ()
			: m_pd3dDevice(nullptr)
			, m_pd3dDeviceContext(nullptr)
			, c_sampleCount(1)
		{ }

		ID3D11RenderTargetView * CreateRenderTarget (IDXGISwapChain1 * chain);
		IDXGISwapChain1 * CreateSwapChain (HWND hwnd);
		bool AssociateFactory ();
		HRESULT CreateDeviceD3D (HWND hWnd);
		IDXGIFactory2 * GetFactory2 ();
		void CleanupDeviceD3D ();
		void CleanupDevice ();
		void OnResize (unsigned w, unsigned h);
		void Render ();
	};
}

