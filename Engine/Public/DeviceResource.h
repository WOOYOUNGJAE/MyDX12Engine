#pragma once
#include "Base.h"

NAMESPACE_(Engine)

class CDeviceResource : public CBase
{
	DECLARE_SINGLETON(CDeviceResource)
	friend class CPipelineManager;
	friend class CRenderer;
public:
	CDeviceResource();
	virtual ~CDeviceResource() override = default;

public: // Init
	HRESULT Init_Graphic_Device(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode,
		_uint iWinCX, _uint iWinCY, _Inout_ ID3D12Device** ppDevice);
	HRESULT Init_Adapter();
	HRESULT Init_CommandObjects();
	HRESULT Init_SwapChain(GRAPHIC_DESC::WINMODE eWinMode);
	// RenderTargetView, Get_DepthStencilViewHeapStart
	HRESULT Create_DescriptorHeap();
	HRESULT Create_CbvSrvUavDescriptorHeap(UINT iNumDescriptors);
public: // LifeCycle
	HRESULT Flush_CommandQueue(const QUEUE_FLUSH_DESC* queue_flush_desc);
	HRESULT Flush_CommandQueue();
	virtual HRESULT Free() override;
public:
	HRESULT On_Resize();
	HRESULT Reset_CmdList();
	HRESULT Close_CmdList();
	void Execute_CmdList();
public: // Getter
	ID3D12Device* Get_Device() { return m_pDevice.Get(); }
#ifdef DXR_ON
	ID3D12Device5* Get_Device5() { return m_pDevice.Get(); }
#endif
	ID3D12GraphicsCommandList* Get_CommandList() { return m_pCommandList.Get(); }
	ID3D12CommandQueue* Get_CommandQueue() { return m_pCommandQueue.Get(); }
	ID3D12Resource* CurrentBackBuffer() { return m_pRenderTargets[m_iCurrBackBuffer].Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE Get_DepthStencilViewHeapStart()const { return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart(); };
	D3D12_GPU_DESCRIPTOR_HANDLE Get_CbvSrvUavHeapStart_GPU() { return m_pCbvSrvUavHeap.Get()->GetGPUDescriptorHandleForHeapStart(); }
	D3D12_CPU_DESCRIPTOR_HANDLE Get_CbvSrvUavHeapStart_CPU() { return m_pCbvSrvUavHeap.Get()->GetCPUDescriptorHandleForHeapStart(); }
	ID3D12DescriptorHeap* Get_CbvSrvUavHeap() { return m_pCbvSrvUavHeap.Get(); }
	IDXGIAdapter1* Get_Adapter() { return m_pAdapter.Get(); }
	UINT Get_RtvDescriptorSize() const { return m_iRtvDescriptorSize; }
	UINT Get_CbvSrvUavDescriptorSize() const { return m_iCbvSrvUavDescriptorSize; }
	UINT* Get_NextCbvSrvUavHeapOffsetPtr() { return &m_iNextCbvSrvUavHeapOffset; }
private: // ComPtr
	ComPtr<IDXGIAdapter1> m_pAdapter;
	ComPtr<IDXGIFactory4> m_pDxgi_Factory = nullptr;
#ifdef DXR_ON
	ComPtr<ID3D12Device5> m_pDevice = nullptr;
#else
	ComPtr<ID3D12Device> m_pDevice = nullptr;
#endif
	ComPtr<ID3D12CommandQueue> m_pCommandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;
private: // SwapChain
	ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr;
	UINT m_iBufferIndex = 0;
private: // Fence
	ComPtr<ID3D12Fence> m_pFence = nullptr;
	HANDLE m_fenceEvent = nullptr;
	UINT64 m_iFenceValue = 0;
private: // Descriptor
	UINT m_iRtvDescriptorSize = 0;
	UINT m_iDsvDescriptorSize = 0;
	UINT m_iCbvSrvUavDescriptorSize = 0;
	UINT m_iNextCbvSrvUavHeapOffset = 0;
private: // Descriptor Heap
	static const int m_iSwapChainBufferCount = 2; // 더블 버퍼링때문에 2로 초기화
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_pCbvSrvUavHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap = nullptr;
	ComPtr<ID3D12Resource> m_pRenderTargets[m_iSwapChainBufferCount]{};
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr;
private:
	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_ScissorRect;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
private: // From Client
	HWND m_hWnd = nullptr;
	_uint m_iClientWinCX = 0;
	_uint m_iClientWinCY = 0;
public:
	FLOAT m_fAspectRatio = 0.f; // Renderer과 메쉬버텍스 위치에 사용
private: // Current
	UINT64 m_iCurFenceVal = 0;
	_uint m_iCurrBackBuffer = 0;
private: // adapter
	UINT                                                m_adapterID;
	std::wstring                                        m_adapterDescription;
};

_NAMESPACE
