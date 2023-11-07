#include "Graphic_Device.h"

IMPLEMENT_SINGLETON(CGraphic_Device)

CGraphic_Device::CGraphic_Device()
{
}

HRESULT CGraphic_Device::Init_Graphic_Device(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode, _uint iWinCX, _uint iWinCY,
	ID3D12Device** ppDevice)
{
	CreateDXGIFactory1(IID_PPV_ARGS(&m_pDxgi_Factory));

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

	if (FAILED(Init_Fence()))
	{
		MSG_BOX("Failed To Create Fence");
		return E_FAIL;
	}

	// Skip 4X MSAA

	if (FAILED(Init_CommandObjects()))
	{
		MSG_BOX("Failed To Create Commad Objects");
		return E_FAIL;
	}

	if (FAILED(Init_SwapChain(hWnd, eWinMode, iWinCX, iWinCY)))
	{
		MSG_BOX("Failed to Create Swap Chain");
		return E_FAIL;
	}
	
	return S_OK;
}

HRESULT CGraphic_Device::Init_Fence()
{
	HRESULT hr = S_OK;

	hr = FAILED(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));

	m_iRtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_iDsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_iCbvSrvUavDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return hr;
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

HRESULT CGraphic_Device::Init_SwapChain(HWND hWnd, GRAPHIC_DESC::WINMODE eWinMode, _uint iWinCX, _uint iWinCY)
{
	/* 스왑체인을 생성한다. = 텍스쳐를 생성하는 행위 + 스왑하는 형태  */
#pragma region SwapChainDesc
	DXGI_SWAP_CHAIN_DESC		SwapChain;
	ZeroMemory(&SwapChain, sizeof(DXGI_SWAP_CHAIN_DESC));

	/*텍스쳐(백버퍼)를 생성하는 행위*/
	SwapChain.BufferDesc.Width = iWinCX;
	SwapChain.BufferDesc.Height = iWinCY;


	SwapChain.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChain.BufferCount = 1;

	/*스왑하는 형태*/
	SwapChain.BufferDesc.RefreshRate.Numerator = 60;
	SwapChain.BufferDesc.RefreshRate.Denominator = 1;
	SwapChain.SampleDesc.Quality = 0;
	SwapChain.SampleDesc.Count = 1;

	SwapChain.OutputWindow = hWnd;
	SwapChain.Windowed = eWinMode;
	SwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
#pragma endregion SwapChainDesc

	/* 백버퍼라는 텍스쳐를 생성했다. */
	if (FAILED(m_pDxgi_Factory->CreateSwapChain(m_pCommandQueue.Get(), &SwapChain, m_pSwapChain.GetAddressOf())))
		return E_FAIL;

	return S_OK;
}

HRESULT CGraphic_Device::Init_DescriptorHeap()
{
	// Descriptor의 관리를 위한 D-Heap생성

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_iSwapChainBufferCount;\
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

HRESULT CGraphic_Device::Free()
{
	return S_OK;
}
