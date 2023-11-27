#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class CGraphic_Device : public CBase
{
	DECLARE_SINGLETON(CGraphic_Device)
	friend class CPipelineManager;
public:
	CGraphic_Device();
	virtual ~CGraphic_Device() override = default;

public: // Init
	HRESULT Init_Graphic_Device(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode,
		_uint iWinCX, _uint iWinCY, _Inout_ ID3D12Device** ppDevice);
	HRESULT Init_CommandObjects();
	HRESULT Init_SwapChain(GRAPHIC_DESC::WINMODE eWinMode);
	// RenderTargetView, DepthStencilView
	HRESULT Create_RTV_DSV_DescriptorHeap();
	HRESULT Init_RenderTargetView();

public: // LifeCycle
	HRESULT Flush_CommandQueue();
	virtual HRESULT Free() override;
public:
	HRESULT On_Resize();
public: // Getter
	ComPtr<ID3D12Device> Get_Device() { return m_pDevice.Get(); }
	ComPtr<ID3D12GraphicsCommandList>  Get_CommandList() { return m_pCommandList.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE Get_DepthStenciView() { return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart(); }

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
	ComPtr<ID3D12Resource> m_pSwapChainBuffer[m_iSwapChainBufferCount]{};
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr;
private:
	D3D12_VIEWPORT m_viewportDesc;
	D3D12_RECT m_ScissorRect;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
private: // From Client
	HWND m_hWnd = nullptr;
	_uint m_iClientWinCX = 0;
	_uint m_iClientWinCY = 0;
private: // Current
	UINT64 m_iCurrFence = 0;
	_uint m_iCurrBackBuffer = 0;
};

_NAMESPACE
