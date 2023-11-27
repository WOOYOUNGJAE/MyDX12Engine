#include "PipelineManager.h"

#include "ComponentManager.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Shader.h"
#include "Graphic_Device.h"
#include "Transform.h"
#include "Camera.h"
IMPLEMENT_SINGLETON(CPipelineManager)

_uint g_iNumFrameResource = 3; // extern frame resource num;
CPipelineManager::CPipelineManager() :
m_pComponentManager(CComponentManager::Get_Instance()),
m_pGameObjectManager(CGameObjectManager::Get_Instance())
{
	Safe_AddRef(m_pComponentManager);
	Safe_AddRef(m_pGameObjectManager);
}

CPipelineManager::FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
{
	if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf()))))
	{
		MSG_BOX("FrameResource : CommandAllocator Failed");
		return;
	}
	m_pPassCB = CUploadBuffer<PassConstants>::Create(device, passCount, true);
	m_pObjectCB = CUploadBuffer<ObjectConstants>::Create(device, objectCount, true);
}

HRESULT CPipelineManager::Initialize()
{
	HRESULT hr = S_OK;
	// GraphicDevice들의 멤버들이 초기화 된 후 참조
	m_pGraphic_Device = CGraphic_Device::Get_Instance();
	m_pDevice = m_pGraphic_Device->Get_Device();
	m_pCommandList= m_pGraphic_Device->Get_CommandList();
	Safe_AddRef(m_pGraphic_Device);

	// Build Root Signiture
	hr = Init_RootSignature();
	if (FAILED(hr))
	{
		return hr;
	}

	hr = Init_FrameResource();
	if (FAILED(hr))
	{
		return hr;
	}


	// Create ConstantBufferView Descriptor Heap
	{
		_uint iNumObj = m_pGameObjectManager->Get_ObjPrototypeMap().size();

		// 각 FrameResource의 물체마다 하나씩 CBV서술자 필요. +1은 Pass CBV 위한 것
		_uint iNumDescriptors = (iNumObj + 1) * g_iNumFrameResource;

		// Pass별 CBV의 시작 오프셋 지정
		m_iPassCBVOffset = iNumObj * g_iNumFrameResource;

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = iNumDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		hr = m_pDevice->CreateDescriptorHeap(&cbvHeapDesc,
			IID_PPV_ARGS(&m_pCbvHeap));
		if (FAILED(hr))
		{
			return hr;
		}
	}

	hr = Init_ConstantBuffersView();
	if (FAILED(hr))
	{
		return hr;
	}


	// Build PSO
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
		pso_desc.InputLayout = { m_vecInputLayout[InputLayout_DEFAULT].data(), (UINT)m_vecInputLayout[InputLayout_DEFAULT].size() };
		pso_desc.pRootSignature = m_RootSigArr[RootSig_DEFAULT].Get();
		// VertexShader ByteCode
		CShader* pShader = dynamic_cast<CShader*>(m_pComponentManager->Find_Prototype(L"Shader_Default"));
		ComPtr<ID3DBlob>  byteCode = pShader->Get_ByteCode(CShader::TYPE_VERTEX);
		pso_desc.VS =
		{
			reinterpret_cast<BYTE*>(byteCode->GetBufferPointer()),
			byteCode->GetBufferSize()
		};
		byteCode = pShader->Get_ByteCode(CShader::TYPE_PIXEL);
		pso_desc.PS =
		{
			reinterpret_cast<BYTE*>(byteCode->GetBufferPointer()),
			byteCode->GetBufferSize()
		};
		pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		pso_desc.SampleMask = UINT_MAX; // 그 어떤 표본도 비활성화 하지 않음
		pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pso_desc.NumRenderTargets = 1;
		pso_desc.RTVFormats[0] = m_pGraphic_Device->m_BackBufferFormat;
		pso_desc.SampleDesc.Count = 1;
		pso_desc.SampleDesc.Quality = 0;
		pso_desc.DSVFormat = m_pGraphic_Device->m_DepthStencilFormat;

		hr = Build_PSO(pso_desc, ENUM_PSO::PSO_DEFAULT);
		if (FAILED(hr))
		{
			MSG_BOX("Pipeline : Build PSO Failed ");
			return hr;
		}
		
	}
	return hr;
}

HRESULT CPipelineManager::Init_FrameResource()
{
	HRESULT hr = S_OK;
	m_vecFrameResource.reserve(g_iNumFrameResource);
	for (int i = 0; i < g_iNumFrameResource; ++i)
	{
		FrameResource* pFrameResource = new FrameResource(m_pDevice.Get(), 1, 0/*TODO:RenderItemSize*/);
		m_vecFrameResource.push_back(pFrameResource);
	}

	return hr;
}

HRESULT CPipelineManager::Init_ConstantBuffersView()
{
	//m_vecFrameResource
	_uint objCBByteSize =  CDevice_Utils::ConstantBufferByteSize(sizeof(ObjectConstants));

	_uint objCount = (_uint)CGameObjectManager::Get_Instance()->Get_ObjPrototypeMap().size();
	// 0번에서 n-1번까지의 서술자들은 0번 FrameResource를 위한 물체별 CBV을 담고,
	// n~2n-1은 1번 FrameResource를 위한 물체별 CBV,
	// 2n~3n-1은 2번 FrameResource를 위한 물체별 CBV, 3n, 3n+1, 3n+2는 각 FrameResource의 Pass CBV를 담음
	for (int frameIndex = 0; frameIndex < g_iNumFrameResource; ++frameIndex)
	{
		auto objectCB = m_vecFrameResource[frameIndex]->m_pObjectCB->Get_UploadBuffer();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB/*Resource*/->GetGPUVirtualAddress();

			// 현재 버퍼에서 i번째 물체별 상수 버퍼의 오프셋
			cbAddress += i * objCBByteSize; // 바이트 단위

			// 서술자 힙에서 i번째 물체별 상수 버퍼의 오프셋
			int heapIndex = frameIndex * objCount + i; // 인덱스
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pCbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_iCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_pDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}
	_uint passCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(PassConstants));
	// 마지막 세 서술자는 FrameResource의 Pass CBV
	for (_uint frameIndex = 0; frameIndex < g_iNumFrameResource; ++frameIndex)
	{
		auto passCB = m_vecFrameResource[frameIndex]->m_pPassCB->Get_UploadBuffer();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		// 서술자 힙 안에서 Pass CBV의 오프셋
		int heapIndex = m_iPassCBVOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pCbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_iCbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_pDevice->CreateConstantBufferView(&cbvDesc, handle);
	}

	//================================================================
#pragma region CBVLegacy
	//m_pUploadBuffer_Constant = CUploadBuffer<_float4x4>::Create(m_pDevice.Get(), 1, true);

	//if (m_pUploadBuffer_Constant == nullptr)
	//{
	//	return E_FAIL;
	//}

	////UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(_float4x4));

	//// 0번째 상수 버퍼의 시작 주소
	//D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_pUploadBuffer_Constant->Get_UploadBuffer()->GetGPUVirtualAddress();
	//// Offset to the ith object constant buffer in the buffer.
	//// 버퍼에 담긴 i번째 상수 버퍼의 오프셋
	//int boxCBufIndex = 0;
	//cbAddress += boxCBufIndex * objCBByteSize;


	//D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	//cbvDesc.BufferLocation = cbAddress;
	//cbvDesc.SizeInBytes = CDevice_Utils::ConstantBufferByteSize(sizeof(_float4x4));

	//m_pDevice->CreateConstantBufferView(
	//	&cbvDesc,
	//	m_pCbvHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion
	return S_OK;
}

HRESULT CPipelineManager::Init_RootSignature()
{
	// RootSig_DEFAULT

	// CBV 하나를 담는 서술자 테이블 생성, 현재 Default는 ConstantObject, ConstantPass로 총 두개
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	1, 0);
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	1, 1/*인덱스*/);

	// 루트 시그니쳐는 테이블이거나 루트 서술자 또는 루트 상수이다.
	CD3DX12_ROOT_PARAMETER slotRootParameterArr[2];

	slotRootParameterArr[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameterArr[1].InitAsDescriptorTable(1, &cbvTable1);

	// 루트 시그니쳐는 루트 매개변수들의 배열
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		2, 
		slotRootParameterArr,
		0, 
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 상수 버퍼 하나로 구성된 서술자 구간을 가리키는, 슬롯하나로 이루어진 루트 시그니쳐 생성
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (errorBlob)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	if (FAILED(m_pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_RootSigArr[RootSig_DEFAULT]))))
	{
		return E_FAIL;
	}

	return S_OK;
}


void CPipelineManager::Pipeline_Tick(_float fDeltaTime)
{
	for (auto& iter : m_vecPipelineLayerArr)
	{
		for (auto& innerIter : iter)
		{
			innerIter->Pipeline_Tick();
		}
	}

	// 순환적으로 자원 배열의 다음 원소 접근
	m_iCurFrameResourceIndex = (m_iCurFrameResourceIndex + 1) % g_iNumFrameResource;
	m_pCurFrameResource = m_vecFrameResource[m_iCurFrameResourceIndex];

	// 1. GPU가 현재 프레임 자원 명령을 처리했는지 확인.
	// 처리하지 못했다면 GPU가 이 울타리 지점 명령들을 처리할 때까지 기다림, Render끝나면 현재Fence 증가
	if (m_pCurFrameResource->Fence != 0 && m_pGraphic_Device->m_pFence->GetCompletedValue() < m_pCurFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		
		// 지정된 Fence에 도달하면 Handle에 신호 보내도록 이벤트 형식으로 등록
		m_pGraphic_Device->m_pFence->SetEventOnCompletion(m_pCurFrameResource->Fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE); // 핸들이 신호 받을때까지 기다리기
		CloseHandle(eventHandle);
	}

	Update_ObjectCBs(fDeltaTime);
	Update_MainPassCB(fDeltaTime);
}

void CPipelineManager::Update_ObjectCBs(_float fDeltaTime)
{
	auto curObjectCB = m_pCurFrameResource->m_pObjectCB;
	for (auto& pair : m_pGameObjectManager->Get_ObjPrototypeMap())
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (pair.second->Get_NumFrameDirtyRef() > 0)
		{
			_matrix world = pair.second->Get_WorldMatrix();

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

			curObjectCB->CopyData(0, objConstants);

			// Next FrameResource need to be updated too.
			--pair.second->Get_NumFrameDirtyRef();
		}
	}
}

void CPipelineManager::Update_MainPassCB(_float fDeltaTime)
{
	// 메인 Pass ConstantView업데이트 후 CurFrameResource에 복사
	if (m_pActiveCam == nullptr)
	{
		return;
	}

	XMMATRIX view = XMLoadFloat4x4(&m_pipelineMatrix[ENUM_PIPELINE_MAT::VIEW_MAT]);
	XMMATRIX proj =  XMLoadFloat4x4(&m_pipelineMatrix[ENUM_PIPELINE_MAT::PROJ_MAT]);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMMATRIX invProj = XMMatrixInverse(nullptr, proj);
	XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);


	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_MainPassCB.EyePosW = m_pActiveCam->Get_Pos();
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float) m_pGraphic_Device->m_iClientWinCX, (float)m_pGraphic_Device->m_iClientWinCY);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_pGraphic_Device->m_iClientWinCX, 1.0f / m_pGraphic_Device->m_iClientWinCY);
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;
	m_MainPassCB.TotalTime = 0;
	m_MainPassCB.DeltaTime = 0;

	auto currPassCB = m_pCurFrameResource->m_pPassCB;
	currPassCB->CopyData(0, m_MainPassCB);
}

HRESULT CPipelineManager::Free()
{
	for (auto& iter : m_vecFrameResource)
	{
		Safe_Delete(iter);
	}
	Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pComponentManager);
	Safe_Release(m_pGraphic_Device);
	return S_OK;
}

HRESULT CPipelineManager::Build_PSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipeline_desc, ENUM_PSO psoIndex)
{
	HRESULT hr = S_OK;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = pipeline_desc;
	hr = m_pDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&(m_PSOArr[psoIndex])));
	if (FAILED(hr))
	{
		MSG_BOX("Failed to Create PSO");
		return hr;
	}

	return hr;
}
