#include "Graphic_Device.h"
#include "PipelineManager.h"

IMPLEMENT_SINGLETON(CGraphic_Device)

CGraphic_Device::CGraphic_Device()
{
	
}

HRESULT CGraphic_Device::Init_Graphic_Device(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode, _uint iWinCX, _uint iWinCY,
	ID3D12Device** ppDevice)
{
	m_hWnd = hWnd;

	m_iClientWinCX = iWinCX;
	m_iClientWinCY = iWinCY;

	// viewport
	m_viewportDesc.TopLeftX = 0;
	m_viewportDesc.TopLeftY = 0;
	m_viewportDesc.Width = static_cast<float>(m_iClientWinCX);
	m_viewportDesc.Height = static_cast<float>(m_iClientWinCY);
	m_viewportDesc.MinDepth = 0.0f;
	m_viewportDesc.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, static_cast<LONG>(m_iClientWinCX), static_cast<LONG>(m_iClientWinCY )};

	CreateDXGIFactory1(IID_PPV_ARGS(&m_pDxgi_Factory));

#ifdef _DEBUG // DirectX Debug Layer

	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
#endif

	// Only Hardware Adapter, No Wrap Adapter
	if (FAILED(D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_pDevice)
	)))
	{
		MSG_BOX("Failed To Create Device");
		return E_FAIL;
	}

	// Init Fence
	if (FAILED(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
	{
		MSG_BOX("Failed To Create Fence");
		return E_FAIL;
	}

	m_iRtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_iDsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_iCbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Skip 4X MSAA

	if (FAILED(Init_CommandObjects()))
	{
		MSG_BOX("Failed To Create Commad Objects");
		return E_FAIL;
	}

	if (FAILED(Init_SwapChain(eWinMode)))
	{
		MSG_BOX("Failed to Create Swap Chain");
		return E_FAIL;
	}

	if (FAILED(Create_RTV_DSV_DescriptorHeap()))
	{
		MSG_BOX("Failed to Create Descriptor Heap");
		return E_FAIL;
	}

	if (FAILED(On_Resize()))
	{
		MSG_BOX("On_Resize Failed");
		return E_FAIL;
	}
	
	return S_OK;
}

HRESULT CGraphic_Device::Init_CommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_pCmdAllocator.GetAddressOf()))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT, // 나중에 Bundle 사용할 수도
		m_pCmdAllocator.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(m_pCommandList.GetAddressOf()))))
	{
		return E_FAIL;
	}

	// 닫힌 상태로 시작
	m_pCommandList->Close();

	return S_OK;
}

HRESULT CGraphic_Device::Init_SwapChain(GRAPHIC_DESC::WINMODE eWinMode)
{
	/* 스왑체인을 생성한다. = 텍스쳐를 생성하는 행위 + 스왑하는 형태  */
	m_pSwapChain.Reset();
#pragma region SwapChainDesc
	DXGI_SWAP_CHAIN_DESC		SwapChain;
	ZeroMemory(&SwapChain, sizeof(DXGI_SWAP_CHAIN_DESC));

	/*텍스쳐(백버퍼)를 생성하는 행위*/
	SwapChain.BufferDesc.Width = m_iClientWinCX;
	SwapChain.BufferDesc.Height = m_iClientWinCY;


	SwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChain.BufferCount = m_iSwapChainBufferCount;

	/*스왑하는 형태*/
	SwapChain.BufferDesc.RefreshRate.Numerator = 60;
	SwapChain.BufferDesc.RefreshRate.Denominator = 1;
	SwapChain.SampleDesc.Quality = 0;
	SwapChain.SampleDesc.Count = 1;

	SwapChain.OutputWindow = m_hWnd;
	SwapChain.Windowed = eWinMode;
	SwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // DX11d은 그냥 Discard
	SwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#pragma endregion SwapChainDesc


	/* 백버퍼라는 텍스쳐를.. 생성했다. */
	if (FAILED(m_pDxgi_Factory->CreateSwapChain(m_pCommandQueue.Get(), &SwapChain, m_pSwapChain.GetAddressOf())))
		return E_FAIL;

	return S_OK;
}

HRESULT CGraphic_Device::Create_RTV_DSV_DescriptorHeap()
{
	// Descriptor의 관리를 위한 D-Heap생성
	// SwapChainBufferCount개 (스왑체인 버퍼)서술자를 담는 RTV힙
	// (뎁스스텐실버퍼)서술자를 담는 DSV힙 하나 생성

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_iSwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	if (FAILED(m_pDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_pRtvHeap.GetAddressOf()))))
	{
		return E_FAIL;
	}


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	if (FAILED(m_pDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_pDsvHeap.GetAddressOf()))))
	{
		return E_FAIL;
	}

	

	return S_OK;
}

HRESULT CGraphic_Device::Init_RenderTargetView()
{


	return S_OK;
}

HRESULT CGraphic_Device::Flush_CommandQueue()
{
	// Fence Point 업데이트
	++m_iCurrFence;

	return S_OK;
}

HRESULT CGraphic_Device::Free()
{
	if (m_pDevice)
	{
		// GPU의 자원을 해제하기 전에 명령 대기열 비워야 안전
		Flush_CommandQueue();
	}
	return S_OK;
}

HRESULT CGraphic_Device::On_Resize()
{
	// 창 resize 시 필요한 정보들 재정의

	if (m_pDevice == nullptr || m_pSwapChain == nullptr
		|| m_pCmdAllocator == nullptr)
	{
		return E_FAIL;
	}

	//
	Flush_CommandQueue();

	if (FAILED(m_pCommandList->Reset(m_pCmdAllocator.Get(), nullptr)))
	{
		return E_FAIL;
	}

	// 기존 RTV, DSV 버퍼 리셋
	for (int i = 0; i < m_iSwapChainBufferCount; ++i)
	{
		m_pSwapChainBuffer[i].Reset();
	}
	m_pDepthStencilBuffer.Reset();

	// 후면 버퍼 재정의
	if (FAILED(m_pSwapChain->ResizeBuffers(
	m_iSwapChainBufferCount,
		m_iClientWinCX, m_iClientWinCY,
		DXGI_FORMAT_R8G8B8A8_UNORM /*BackBufferFormat*/,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
	{
		return E_FAIL;
	}

	m_iCurrBackBuffer = 0;

	// 새 창크기에 맞게 RTV 재생성
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (_int i = 0; i < m_iSwapChainBufferCount; ++i)
	{
		if (FAILED(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainBuffer[i]))))
		{
			return E_FAIL;
		}
		m_pDevice->CreateRenderTargetView(m_pSwapChainBuffer[i].Get(), nullptr,
			rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_iRtvDescriptorSize);
	}

	// Create Depth Stencil Buffer
	{
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = m_iClientWinCX;
		depthStencilDesc.Height = m_iClientWinCY;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;

		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

		//depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		//depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;


		CD3DX12_HEAP_PROPERTIES tempProperties(D3D12_HEAP_TYPE_DEFAULT);

		if (FAILED(m_pDevice->CreateCommittedResource(
			&tempProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(m_pDepthStencilBuffer.GetAddressOf()
		))))// 메모리 할당된 리소스
		{
			return E_FAIL;
		}
	}

	// Create Depth Stencil Buffer
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &dsvDesc, Get_DepthStenciView());
	}

	CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	// 자원을 초기 상태에서 깊이 버퍼를 사용할 수 있는 상태로 전이
	m_pCommandList->ResourceBarrier(1, &tempBarrier);

	if (FAILED(m_pCommandList->Close()))
	{
		return E_FAIL;
	}

	// Resize Command
	ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
	// 명령 목록을 GPU 명령 대기열에 추가
	m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	Flush_CommandQueue();

	// viewport
	m_viewportDesc.TopLeftX = 0;
	m_viewportDesc.TopLeftY = 0;
	m_viewportDesc.Width = static_cast<float>(m_iClientWinCX);
	m_viewportDesc.Height = static_cast<float>(m_iClientWinCY);
	m_viewportDesc.MinDepth = 0.0f;
	m_viewportDesc.MaxDepth = 1.0f;

	m_ScissorRect = { 0, 0, static_cast<LONG>(m_iClientWinCX), static_cast<LONG>(m_iClientWinCY) };

	return S_OK;
}