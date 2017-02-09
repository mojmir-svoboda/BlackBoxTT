#include "DX11.h"
#include <blackbox/common.h>
#include <functional>

namespace bb {

	ID3D11RenderTargetView * DX11::CreateRenderTarget (IDXGISwapChain1 * chain)
	{
		ID3D11RenderTargetView * view = nullptr;

		ID3D11Texture2D * pBackBuffer;
		D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
		ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));
		render_target_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		HRESULT hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &view);
		if (!SUCCEEDED(hr))
		{
			Assert(view);
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "DX11 device CreateRenderTargetView failed, hresult=0x%x", hr);
			pBackBuffer->Release();
			return nullptr;
		}

		m_pd3dDeviceContext->OMSetRenderTargets(1, &view, NULL);
		pBackBuffer->Release();
		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT, "Created render target @ 0x%x for chain @ 0x%x", view, chain);
		return view;
	}

	IDXGISwapChain1 * DX11::CreateSwapChain (HWND hwnd)
	{
		IDXGISwapChain1 * chain = nullptr;

		IDXGIDevice * dxgiDevice = nullptr;
		HRESULT hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)& dxgiDevice);
		scope_guard_t on_exit_dxgid = mkScopeGuard(std::mem_fun(&IDXGIDevice::Release), dxgiDevice);
		if (!SUCCEEDED(hr))
		{
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "DX11 device QueryInteface for dxgi device failed, hresult=0x%x", hr);
			on_exit_dxgid.Dismiss();
			return nullptr;
		}

		IDXGIAdapter * dxgiAdapter = nullptr;
		hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)& dxgiAdapter);
		scope_guard_t on_exit_dxgia = mkScopeGuard(std::mem_fun(&IDXGIDevice::Release), dxgiAdapter);
		if (!SUCCEEDED(hr))
		{
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "cannot find parent of dxgi device, hresult=0x%x", hr);
			on_exit_dxgia.Dismiss();
			return nullptr;
		}

		IDXGIFactory * dxgiFactory = nullptr;
		hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void **)& dxgiFactory);
		scope_guard_t on_exit_dxgif = mkScopeGuard(std::mem_fun(&IDXGIDevice::Release), dxgiFactory);
		if (!SUCCEEDED(hr))
		{
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "cannot find parent of dxgi adapter, hresult=0x%x", hr);
			on_exit_dxgif.Dismiss();
			return nullptr;
		}

		IDXGIFactory2 * dxgiFactory2 = nullptr;
		hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
		scope_guard_t on_exit_dxgif2 = mkScopeGuard(std::mem_fun(&IDXGIDevice::Release), dxgiFactory2);
		if (SUCCEEDED(hr))
		{
			// This system has DirectX 11.1 or later installed, so we can use this interface
			DXGI_SWAP_CHAIN_DESC1 stSwpChainDesc = { 0 };
			stSwpChainDesc.Width = 0;
			stSwpChainDesc.Height = 0;
			stSwpChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			stSwpChainDesc.Stereo = false;
			stSwpChainDesc.SampleDesc.Count = 1;
			stSwpChainDesc.SampleDesc.Quality = 0;
			stSwpChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			stSwpChainDesc.BufferCount = 1;                                 // Used to be 1
			stSwpChainDesc.Scaling = DXGI_SCALING_STRETCH;
			stSwpChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			stSwpChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, hwnd, &stSwpChainDesc, nullptr, nullptr, &chain);
			dxgiFactory2->Release();
		}
		else
		{
			// This system only has DirectX 11.0 installed
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "cannot find DXGIFactory2 interface, hresult=0x%x", hr);
			on_exit_dxgif2.Dismiss();
			//dxgiFactory->CreateSwapChain( /* parameters */);
		}

		TRACE_MSG(LL_INFO, CTX_BB | CTX_GFX | CTX_INIT, "Created DX11 swap chain @ 0x%x for hwnd=0x%x", chain, hwnd);
		return chain;
	}

	HRESULT DX11::CreateDeviceD3D (HWND hWnd)
	{
		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_1 };
		D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		UINT createDeviceFlags = 0;
#if defined(_DEBUG)
		// If the project is in a debug build, enable the debug layer.
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT; // enable render on surfaces via d2d
		HRESULT hr = D3D11CreateDevice( nullptr, md3dDriverType, nullptr, createDeviceFlags, nullptr, 0,
				D3D11_SDK_VERSION, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
		if (hr != S_OK)
		{
			TRACE_MSG(LL_ERROR, CTX_BB | CTX_GFX | CTX_INIT, "D3D11CreateDevice failed, hresult=0x%x", hr);
			return E_FAIL;
		}

		// Setup rasterizer
		{
			D3D11_RASTERIZER_DESC RSDesc;
			memset(&RSDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
			RSDesc.FillMode = D3D11_FILL_SOLID;
			RSDesc.CullMode = D3D11_CULL_NONE;
			RSDesc.FrontCounterClockwise = FALSE;
			RSDesc.DepthBias = 0;
			RSDesc.SlopeScaledDepthBias = 0.0f;
			RSDesc.DepthBiasClamp = 0;
			RSDesc.DepthClipEnable = TRUE;
			RSDesc.ScissorEnable = TRUE;
			RSDesc.AntialiasedLineEnable = FALSE;
			RSDesc.MultisampleEnable = (c_sampleCount > 1) ? TRUE : FALSE;

			ID3D11RasterizerState* pRState = NULL;
			m_pd3dDevice->CreateRasterizerState(&RSDesc, &pRState);
			m_pd3dDeviceContext->RSSetState(pRState);
			pRState->Release();
		}

		return S_OK;
	}

	void DX11::CleanupDeviceD3D ()
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "DX11 cleanup");
		if (m_pd3dDeviceContext) { m_pd3dDeviceContext->Release(); m_pd3dDeviceContext = NULL; }
		if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = NULL; }
	}

	void DX11::OnResize (unsigned w, unsigned h)
	{
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "DX11::OnResize");
	}

}

