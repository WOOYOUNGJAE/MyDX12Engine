#include "Renderer.h"
#include "Graphic_Device.h"
#include "PipelineManager.h"
#include "GameObject.h"

CRenderer* CRenderer::Create()
{
	CRenderer* pInstance = new CRenderer();

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("CRenderer : Failed to Init Prototype");
	}

	return pInstance;
}

CComponent* CRenderer::Clone(void* pArg)
{
	CBase::AddRef();
	return this;
}

HRESULT CRenderer::Initialize_Prototype()
{
	HRESULT hr = S_OK;
	m_pGraphic_Device = CGraphic_Device::Get_Instance();
	m_pPipelineManager = CPipelineManager::Get_Instance();
	/*m_pCommandAllocator = m_pGraphic_Device->m_pCmdAllocator.Get();
	m_pCommandList = m_pGraphic_Device->m_pCommandList.Get();*/
	m_pCommandQueue = m_pGraphic_Device->m_pCommandQueue.Get(); // CommandQueue는 따로 만들지 않고 공유, 스왑체인이 매치되어야 하기 때문
	m_pRtvHeap = m_pGraphic_Device->m_pRtvHeap.Get();
	m_pRenderTargetArr = m_pGraphic_Device->m_pRenderTargets->GetAddressOf();
	//m_pFence = m_pGraphic_Device->m_pFence.Get();

	ID3D12Device* pDevice = m_pGraphic_Device->m_pDevice.Get();

	// Init Fence
	hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(hr))
	{
		MSG_BOX("Failed To Create Fence");
		return E_FAIL;
	}
	// Fence Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);


	/*D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
	{
		return E_FAIL;
	}*/

	hr = pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pCommandAllocator));
	if (FAILED(hr)) { return E_FAIL; }

	hr = pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT, // 나중에 Bundle 사용할 수도
		m_pCommandAllocator, // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(&m_pCommandList));
	if (FAILED(hr)) { return E_FAIL; }

	// 닫힌 상태로 시작
	m_pCommandList->Close();

#pragma region ConstantBuffer
	// ObjConstantResource
	const UINT iObjConstantBufSize = sizeof(OBJ_CONSTANT_BUFFER);

	hr = pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(iObjConstantBufSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pObjConstantsResource));
	if (FAILED(hr)) { return E_FAIL; }

	// Describe and create a constant buffer view.
	/*UINT& refNextCbvSrvUavHeapOffset = CGraphic_Device::Get_Instance()->m_iNextCbvSrvUavHeapOffset;
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pObjConstantsResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = iObjConstantBufSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(CGraphic_Device::Get_Instance()->Get_CbvSrvUavHeapStart_CPU());
	handle.Offset(1, refNextCbvSrvUavHeapOffset);
	refNextCbvSrvUavHeapOffset += CGraphic_Device::Get_Instance()->m_iCbvSrvUavDescriptorSize;
	pDevice->CreateConstantBufferView(&cbvDesc, handle);*/

	//CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	//hr = (m_pObjConstantsResource->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	//memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
#pragma endregion


	// FrameResource
	for (UINT i = 0; i < g_iNumFrameResource; ++i)
	{
		m_vecFrameResource.push_back(new FrameResource(
			pDevice,
			1/**/,
			0/**/)
		);
	}
	m_pCurFrameResource = m_vecFrameResource[0]; // TODO FrameResource 2이상되면 수정
	// Build Obj Constant Buffer
	UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(OBJ_CONSTANT_BUFFER));
	UINT objCount = 1; //
	UINT iCbvSrvUavDescriptorSize = m_pGraphic_Device->m_iCbvSrvUavDescriptorSize;
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pGraphic_Device->Get_CbvSrvUavHeapStart_CPU());
	m_iCBVHeapStartOffset = *m_pGraphic_Device->Get_NextCbvSrvUavHeapOffsetPtr();
	handle.Offset((INT)m_iCBVHeapStartOffset); // SRV 생성 후 Cbv 힙 시작 오프셋
	for (UINT iFrameIndex = 0; iFrameIndex < g_iNumFrameResource; ++iFrameIndex)
	{
		ID3D12Resource* pObjCB = m_vecFrameResource[iFrameIndex]->pObjectCB->Get_UploadBuffer();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = pObjCB->GetGPUVirtualAddress();

			// 현재 버퍼에서 i번째 물체별 상수 버퍼의 오프셋
			cbAddress += i * objCBByteSize;

			// 서술자 힙에서 i번째 물체별 상수 버퍼의 오프셋

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			pDevice->CreateConstantBufferView(&cbvDesc, handle);

			int heapIndex = iFrameIndex * objCount + i;
			handle.Offset(heapIndex, iCbvSrvUavDescriptorSize);
		}
	}

	m_queue_flush_desc = {
		&m_iFenceValue,
		m_pCommandQueue,
		m_pFence,
		&m_fenceEvent
	};

	return S_OK;
}

HRESULT CRenderer::Initialize(void* pArg)
{
	return CComponent::Initialize(pArg);
}

void CRenderer::Update_ObjCB(CGameObject* pGameObj)
{
	XMMATRIX matWorld = XMLoadFloat4x4(&pGameObj->Get_WorldMatrix());
	
	OBJ_CONSTANT_BUFFER objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(matWorld));

	m_pCurFrameResource->pObjectCB->CopyData(pGameObj->Get_ClonedNum(), objConstants);
}

void CRenderer::BeginRender()
{
	m_pCommandAllocator->Reset();
	m_pCommandList->Reset(m_pCommandAllocator, nullptr);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_iFrameIndex, m_pGraphic_Device->m_iRtvDescriptorSize);
	
	m_pCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargetArr[m_iFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->ClearRenderTargetView(m_pGraphic_Device->CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);

	m_pCommandList->RSSetViewports(1, &m_pGraphic_Device->m_screenViewport);
	m_pCommandList->RSSetScissorRects(1, &m_pGraphic_Device->m_ScissorRect);
	m_pCommandList->OMSetRenderTargets(1, &rtvHeapHandle, FALSE, nullptr);
}

void CRenderer::MainRender()
{
	UINT iTableTypeIndex = 0;
	ID3D12DescriptorHeap* pSrvHeap = m_pGraphic_Device->Get_CbvSrvUavHeap();
	ID3D12DescriptorHeap* ppHeaps[] = { pSrvHeap };
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHandle;
	for (UINT IsFirst = 0; IsFirst < RENDER_PRIORITY_END; ++IsFirst)
	{
		for (UINT eBlendModeEnum = 0; eBlendModeEnum < RENDER_BLENDMODE_END; ++eBlendModeEnum)
		{
			for (UINT eShaderTypeEnum = 0; eShaderTypeEnum < RENDER_SHADERTYPE_END; ++eShaderTypeEnum)
			{
				iTableTypeIndex = eShaderTypeEnum; // shaderType과 RootSigParamType 일관하다고 가정 시, 가능성
				for (UINT eRootsigType = 0; eRootsigType < ROOTSIG_TYPE_END; ++eRootsigType)
				{
					if (m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootsigType].empty())
					{
						continue;
					}

					ID3D12PipelineState* pPSO = m_pPipelineManager->Get_PSO(IsFirst, eBlendModeEnum, eShaderTypeEnum, eRootsigType);
					if (pPSO == nullptr) { continue; }

					cbvSrvUavHandle.InitOffsetted(pSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0);
					m_pCommandList->SetGraphicsRootSignature(m_pPipelineManager->Get_RootSig(eRootsigType)); // RoogSig객체 세팅
					if (eShaderTypeEnum != SHADERTYPE_SIMPLE)
					{
						pSrvHeap = m_pGraphic_Device->Get_CbvSrvUavHeap();
						m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
					}

					m_pCommandList->SetPipelineState(pPSO);

					for (auto& iter : m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootsigType])
					{
						m_pCommandList->IASetPrimitiveTopology(iter->PrimitiveType());
						m_pCommandList->IASetVertexBuffers(0, 1, &iter->VertexBufferView());
						//m_pCommandList->IASetIndexBuffer(&iter->IndexBufferView());

						// Texture
						cbvSrvUavHandle.Offset(iter->Get_CbvSrvUavHeapOffset_Texture());
						m_pCommandList->SetGraphicsRootDescriptorTable(1, cbvSrvUavHandle);

						// ObjCB
						cbvSrvUavHandle.Offset(m_iCBVHeapStartOffset);
						m_pCommandList->SetGraphicsRootDescriptorTable(1, cbvSrvUavHandle);
						Update_ObjCB(iter);



						m_pCommandList->DrawInstanced(3, 1, 0, 0);
						/*m_pCommandList->DrawIndexedInstanced(
							3,
							1,
							0,
							0,
							0);*/
						Safe_Release(iter);
					}
					m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootsigType].clear();
				}
			}
		}
	}
}

void CRenderer::EndRender()
{
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargetArr[m_iFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_pCommandList->Close(); // 기록 중단

	ID3D12CommandList* pCommandListArr[] = {m_pCommandList, };
	m_pCommandQueue->ExecuteCommandLists(_countof(pCommandListArr), pCommandListArr);
}

void CRenderer::Present()
{
	// Swap the back and front buffers
	HRESULT hr = m_pGraphic_Device->m_pSwapChain->Present(0, 0);
	if (FAILED(hr)) { MSG_BOX("Present Failed"); }

	m_iFrameIndex = (m_pGraphic_Device->m_iCurrBackBuffer + 1) % m_pGraphic_Device->m_iSwapChainBufferCount;
	m_pGraphic_Device->m_iCurrBackBuffer = (m_pGraphic_Device->m_iCurrBackBuffer + 1) % m_pGraphic_Device->m_iSwapChainBufferCount;
	
#pragma region Fence and Wait
	CGraphic_Device::Get_Instance()->Flush_CommandQueue(&m_queue_flush_desc);
#pragma endregion
}

HRESULT CRenderer::Free()
{
	CloseHandle(m_fenceEvent);

	Safe_Release(m_pObjConstantsResource);
	Safe_Release(m_pCommandList);
	Safe_Release(m_pCommandAllocator);
	Safe_Release(m_pFence);

	// FrameResource
	for (auto& iter : m_vecFrameResource)
	{
		Safe_Delete(iter);
	}
	m_vecFrameResource.clear();

	for (auto& iter0 : m_RenderGroup)
	{
		for (auto& iter1 : iter0)
		{
			for (auto& iter2 : iter1)
			{
				for (auto& iter3 : iter2)
				{
					for (auto& iter4 : iter3)
					{
						Safe_Release(iter4);						
					}
					iter3.clear();
				}
				
			}
		}
	}

	return CComponent::Free();
}

void CRenderer::AddTo_RenderGroup(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eRootsigTypeEnum,
                                  CGameObject* pGameObject)
{
	m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootsigTypeEnum].push_back(pGameObject);
	Safe_AddRef(pGameObject);
}

//--------------------------------------------------------------------------------------

FrameResource::FrameResource(ID3D12Device * pDevice, UINT iObjectCount, UINT iPassCount)
{
	pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&pCmdListAlloc));

	//PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	pObjectCB = CUploadBuffer<OBJ_CONSTANT_BUFFER>::Create(pDevice, iObjectCount, true);
}

FrameResource::~FrameResource()
{
	Safe_Release(pObjectCB);
	Safe_Release(pCmdListAlloc);
}
