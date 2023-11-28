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
	// GraphicDevice���� ������� �ʱ�ȭ �� �� ����
	m_pGraphic_Device = CGraphic_Device::Get_Instance();
	m_pDevice = m_pGraphic_Device->Get_Device();
	m_pCommandList= m_pGraphic_Device->Get_CommandList();
	Safe_AddRef(m_pGraphic_Device);

	// Build Root Signature
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
		iNumObj = 100; // TODO : ���� ���� ����� �����Ӱ� 100��
		// �� FrameResource�� ��ü���� �ϳ��� CBV������ �ʿ�. +1�� Pass CBV ���� ��
		_uint iNumDescriptors = (iNumObj + 1) * g_iNumFrameResource;

		// Pass�� CBV�� ���� ������ ����
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
		pso_desc.SampleMask = UINT_MAX; // �� � ǥ���� ��Ȱ��ȭ ���� ����
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
	UINT iObjCount = m_pGameObjectManager->Get_ObjPrototypeMap().size(); // TODO : ���� ���� ����� �����Ӱ� 100��
	iObjCount = 100; 
	m_vecFrameResource.reserve(g_iNumFrameResource);
	for (int i = 0; i < g_iNumFrameResource; ++i)
	{
		FrameResource* pFrameResource = new FrameResource(m_pDevice.Get(), 1, iObjCount);
		m_vecFrameResource.push_back(pFrameResource);
	}

	return hr;
}

HRESULT CPipelineManager::Init_ConstantBuffersView()
{
	//m_vecFrameResource
	_uint objCBByteSize =  CDevice_Utils::ConstantBufferByteSize(sizeof(ObjectConstants));

	_uint objCount = (_uint)CGameObjectManager::Get_Instance()->Get_ObjPrototypeMap().size();
	objCount = 100; // TODO : ���� ���� ����� �����Ӱ� 100��
	// 0������ n-1�������� �����ڵ��� 0�� FrameResource�� ���� ��ü�� CBV�� ���,
	// n~2n-1�� 1�� FrameResource�� ���� ��ü�� CBV,
	// 2n~3n-1�� 2�� FrameResource�� ���� ��ü�� CBV, 3n, 3n+1, 3n+2�� �� FrameResource�� Pass CBV�� ����
	for (int frameIndex = 0; frameIndex < g_iNumFrameResource; ++frameIndex)
	{
		auto objectCB = m_vecFrameResource[frameIndex]->m_pObjectCB->Get_UploadBuffer();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB/*Resource*/->GetGPUVirtualAddress();

			// ���� ���ۿ��� i��° ��ü�� ��� ������ ������
			cbAddress += i * objCBByteSize; // ����Ʈ ����

			// ������ ������ i��° ��ü�� ��� ������ ������
			int heapIndex = frameIndex * objCount + i; // �ε���
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pCbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_iCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_pDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}
	_uint passCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(PassConstants));
	// ������ �� �����ڴ� FrameResource�� Pass CBV
	for (_uint frameIndex = 0; frameIndex < g_iNumFrameResource; ++frameIndex)
	{
		auto passCB = m_vecFrameResource[frameIndex]->m_pPassCB->Get_UploadBuffer();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		// ������ �� �ȿ��� Pass CBV�� ������
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

	//// 0��° ��� ������ ���� �ּ�
	//D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_pUploadBuffer_Constant->Get_UploadBuffer()->GetGPUVirtualAddress();
	//// Offset to the ith object constant buffer in the buffer.
	//// ���ۿ� ��� i��° ��� ������ ������
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
	HRESULT hr = S_OK;
	// RootSig_DEFAULT

	// CBV �ϳ��� ��� ������ ���̺� ����, ���� Default�� ConstantObject, ConstantPass�� �� �ΰ�
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	1, 0);
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	1, 1/*�ε���*/);

	// ��Ʈ �ñ״��Ĵ� ���̺��̰ų� ��Ʈ ������ �Ǵ� ��Ʈ ����̴�.
	CD3DX12_ROOT_PARAMETER slotRootParameterArr[2];

	slotRootParameterArr[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameterArr[1].InitAsDescriptorTable(1, &cbvTable1);

	// ��Ʈ �ñ״��Ĵ� ��Ʈ �Ű��������� �迭
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		2, 
		slotRootParameterArr,
		0, 
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// ��� ���� �ϳ��� ������ ������ ������ ����Ű��, �����ϳ��� �̷���� ��Ʈ �ñ״��� ����
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
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

	// ��ȯ������ �ڿ� �迭�� ���� ���� ����
	m_iCurFrameResourceIndex = (m_iCurFrameResourceIndex + 1) % g_iNumFrameResource;
	m_pCurFrameResource = m_vecFrameResource[m_iCurFrameResourceIndex];

	// 1. GPU�� ���� ������ �ڿ� ����� ó���ߴ��� Ȯ��.
	// ó������ ���ߴٸ� GPU�� �� ��Ÿ�� ���� ��ɵ��� ó���� ������ ��ٸ�, Render������ ����Fence ����
	if (m_pCurFrameResource->Fence != 0 && m_pGraphic_Device->m_pFence->GetCompletedValue() < m_pCurFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		
		// ������ Fence�� �����ϸ� Handle�� ��ȣ �������� �̺�Ʈ �������� ���
		m_pGraphic_Device->m_pFence->SetEventOnCompletion(m_pCurFrameResource->Fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE); // �ڵ��� ��ȣ ���������� ��ٸ���
		CloseHandle(eventHandle);
	}

	Update_ObjectCBs(fDeltaTime);
	Update_MainPassCB(fDeltaTime);
}


void CPipelineManager::Update_ObjectCBs(_float fDeltaTime)
{
	auto curObjectCB = m_pCurFrameResource->m_pObjectCB;
	for (auto& layerIter : m_vecPipelineLayerArr)
	{
		for (auto& iter : layerIter)
		{
			if (iter == nullptr)
			{
				break;
			}
			// Only update the cbuffer data if the constants have changed.  
			// This needs to be tracked per frame resource.
			if (iter->Get_NumFrameDirtyRef() > 0)
			{
				_matrix world = iter->Get_WorldMatrix();

				ObjectConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

				curObjectCB->CopyData(0, objConstants);

				// Next FrameResource need to be updated too.
				--iter->Get_NumFrameDirtyRef();
			}
		}
	}
}

void CPipelineManager::Update_MainPassCB(_float fDeltaTime)
{
	// ���� Pass ConstantView������Ʈ �� CurFrameResource�� ����
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

void CPipelineManager::Render()
{
#pragma region ��� ���� �� ����
	auto cmdListAlloc = m_pCurFrameResource->CmdListAlloc;

	// CommandList�� ���� ���Ŀ��� reset ����
	HRESULT hr = cmdListAlloc->Reset();

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	m_pCommandList.Get()->Reset(cmdListAlloc.Get(), m_PSOArr[PSO_DEFAULT].Get());
	int a = 1;
	m_pCommandList->RSSetViewports(1, &m_pGraphic_Device->m_screenViewport);
	m_pCommandList->RSSetScissorRects(1, &m_pGraphic_Device->m_ScissorRect);

	// Indicate a state transition on the resource usage.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pGraphic_Device->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	m_pCommandList->ClearRenderTargetView(m_pGraphic_Device->CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_pCommandList->ClearDepthStencilView(m_pGraphic_Device->DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_pCommandList->OMSetRenderTargets(1, &m_pGraphic_Device->CurrentBackBufferView(), true, &m_pGraphic_Device->DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_pCbvHeap.Get() };
	m_pCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_pCommandList->SetGraphicsRootSignature(m_RootSigArr[RootSig_DEFAULT].Get());

	int passCbvIndex = m_iPassCBVOffset + m_iCurFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pCbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, m_iCbvSrvUavDescriptorSize);
	m_pCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);


#pragma region Draw
	UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(ObjectConstants));

	auto objectCB = m_pCurFrameResource->m_pObjectCB->Get_UploadBuffer();

	// For each render item...
	_uint iTmpObjCBIndex = 0;
	//_uint iNumObject = m_pGameObjectManager->Get_ObjPrototypeMap().size();
	_uint iNumObject = 0;
	for (auto& iter : m_vecPipelineLayerArr)
	{
		iNumObject += iter.size();
	}
	for (auto& layerIter : m_vecPipelineLayerArr)
	{
		for (auto& iter : layerIter)
		{
			auto pObject = iter;

			m_pCommandList.Get()->IASetVertexBuffers(0, 1, &pObject->VertexBufferView());
			m_pCommandList.Get()->IASetIndexBuffer(&pObject->IndexBufferView());
			m_pCommandList.Get()->IASetPrimitiveTopology(pObject->PrimitiveType());

			// Offset to the CBV in the descriptor heap for this object and for this frame resource.
			UINT cbvIndex = m_iCurFrameResourceIndex * iNumObject + iTmpObjCBIndex++;
			auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pCbvHeap->GetGPUDescriptorHandleForHeapStart());
			cbvHandle.Offset(cbvIndex, m_iCbvSrvUavDescriptorSize);

			m_pCommandList.Get()->SetGraphicsRootDescriptorTable(0, cbvHandle);

			m_pCommandList.Get()->DrawIndexedInstanced(
				pObject->Num_Indices(),
				1,
				0,
				0,
				0);
		}
	}

#pragma endregion


	// Indicate a state transition on the resource usage.
	m_pCommandList.Get()->ResourceBarrier(
		1, 
		&CD3DX12_RESOURCE_BARRIER::Transition(m_pGraphic_Device->CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	m_pCommandList.Get()->Close();

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
	m_pGraphic_Device->m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	m_pGraphic_Device->m_pSwapChain->Present(0, 0);
	m_pGraphic_Device->m_iCurrBackBuffer = (m_pGraphic_Device->m_iCurrBackBuffer + 1) % m_pGraphic_Device->m_iSwapChainBufferCount;
#pragma endregion


	// Advance the fence value to mark commands up to this fence point.
	m_pCurFrameResource->Fence = ++m_pGraphic_Device->m_iCurrFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	//  �� �Լ� ���� �������� ��� ó�� �Ŀ� �� ��Ÿ�� ����(mCurrentFence) �����ϵ���
	m_pGraphic_Device->m_pCommandQueue->Signal(m_pGraphic_Device->m_pFence.Get(), m_pGraphic_Device->m_iCurrFence);
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

HRESULT CPipelineManager::Re_Initialize()
{
	HRESULT hr = S_OK;
	return hr;
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

void CPipelineManager::Update_ObjPipelineLayer(CGameObject* pObject, ENUM_PSO ePsoEnum)
{
	for (auto& listIter : m_vecPipelineLayerArr)
	{
		for (auto& objIter = listIter.begin(); objIter != listIter.end(); ++objIter)
		{
			
			if (*objIter == pObject)
			{
				// ���� �־��ְ� ������ �����ֱ�
				m_vecPipelineLayerArr[ePsoEnum].push_back(pObject);
				objIter = listIter.erase(objIter);
				return;
			}
			else
			{
				++objIter;
			}
		}
	}
	// �� ã������ (�ƿ� ó�� ȣ��) �־��ֱ�
	m_vecPipelineLayerArr[ePsoEnum].push_back(pObject);
}
