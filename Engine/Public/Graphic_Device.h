#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class CGraphic_Device : public CBase
{
	DECLARE_SINGLETON(CGraphic_Device)

public:
	CGraphic_Device();
	virtual ~CGraphic_Device() override = default;

public:
	HRESULT Init_Graphic_Device(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode,
		_uint iWinCX, _uint iWinCY, _Inout_ ID3D12Device** ppDevice);
	HRESULT Init_Fence();
	HRESULT Init_CommandObjects();
	HRESULT Init_SwapChain(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode, _uint iWinCX, _uint iWinCY);
	// RenderTargetView, DepthStencilView
	HRESULT Init_DescriptorHeap();
	//HRESULT Init_RenderTargetView();
	virtual HRESULT Free() override;

private: // ComPtr
	ComPtr<IDXGIFactory4> m_pDxgi_Factory = nullptr;
	ComPtr<ID3D12Device> m_pDevice = nullptr;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;
	ComPtr<ID3D12Fence> m_pFence = nullptr;
	ComPtr<ID3D12CommandQueue> m_pCommandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
private: // Descriptor Desc
	UINT m_iRtvDescriptorSize = 0;
	UINT m_iDsvDescriptorSize = 0;
	UINT m_iCbvSrvUavDescriptorSize = 0;
private: // Descriptor Heap
	static const int m_iSwapChainBufferCount = 2; // 더블 버퍼링때문에 2로 초기화
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap = nullptr;
};

_NAMESPACE