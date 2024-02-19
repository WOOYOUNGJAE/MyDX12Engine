#include "Renderer.h"
#include "Camera.h"
#include "CameraManager.h"
#include "Graphic_Device.h"
#include "PipelineManager.h"
#include "GameObject.h"
#include "MyMath.h"

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


	m_queue_flush_desc = {
		&m_iFenceValue,
		m_pCommandQueue,
		m_pFence,
		&m_fenceEvent
	};

	m_fAspectRatio = CGraphic_Device::Get_Instance()->m_fAspectRatio;
	m_mProj = XMMatrixPerspectiveFovLH(0.25f * ::MyMath::Pi, m_fAspectRatio, m_fNear, m_fFar);

	return S_OK;
}

HRESULT CRenderer::Initialize(void* pArg)
{
	return CComponent::Initialize(pArg);
}

HRESULT CRenderer::Build_FrameResource()
{
	HRESULT hr = S_OK;

	ID3D12Device* pDevice = m_pGraphic_Device->m_pDevice.Get();
	// FrameResource
	for (UINT i = 0; i < g_iNumFrameResource; ++i)
	{
		m_vecFrameResource.push_back(new FrameResource(
			pDevice,
			1/**/,
			1/**/)
		);
	}
	m_pCurFrameResource = m_vecFrameResource[0]; // TODO FrameResource 2이상되면 수정

	// Build Obj Constant Buffer
	UINT objCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(OBJ_CONSTANT_BUFFER));
	UINT objCount = 1; //
	UINT iCbvSrvUavDescriptorSize = m_pGraphic_Device->m_iCbvSrvUavDescriptorSize;
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pGraphic_Device->Get_CbvSrvUavHeapStart_CPU());
	m_iObjCBVHeapStartOffset = *m_pGraphic_Device->Get_NextCbvSrvUavHeapOffsetPtr();
	m_iPassCBVHeapStartOffset = m_iObjCBVHeapStartOffset;
	handle.Offset((INT)m_iObjCBVHeapStartOffset); // SRV 생성 후 Cbv 힙 시작 오프셋
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

			INT heapIndex = iFrameIndex * objCount + i;
			handle.Offset(heapIndex, iCbvSrvUavDescriptorSize);
			m_iPassCBVHeapStartOffset += iCbvSrvUavDescriptorSize; // 마지막 ObjCB 생성될 때까지 업데이트
		}
	}
	
	UINT passCBByteSize = CDevice_Utils::ConstantBufferByteSize(sizeof(PASS_CONSTANT_BUFFER));
	handle.InitOffsetted(m_pGraphic_Device->Get_CbvSrvUavHeapStart_CPU(), 0);
	handle.Offset(m_iPassCBVHeapStartOffset);
	// 마지막 세 서술자는 FrameResource의 Pass CBV
	for (INT frameIndex = 0; frameIndex < g_iNumFrameResource; ++frameIndex)
	{
		ID3D12Resource* passCB = m_vecFrameResource[frameIndex]->pPassCB->Get_UploadBuffer(); 
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		// 서술자 힙 안에서 Pass CBV의 오프셋
		INT heapIndex = m_iPassCBVHeapStartOffset + frameIndex;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;
		pDevice->CreateConstantBufferView(&cbvDesc, handle);

		handle.Offset(heapIndex, iCbvSrvUavDescriptorSize);
	}

	return hr;
}

void CRenderer::Update_PassCB()
{
	Matrix mainCamMatInClient = CCameraManager::Get_Instance()->Get_MainCam()->Get_WorldMatrix();
	PASS_CONSTANT_BUFFER passConstants;
	passConstants.mViewMat = mainCamMatInClient.Invert().Transpose();
	passConstants.mProjMat = m_mProj.Transpose();
	m_pCurFrameResource->pPassCB->CopyData(0, passConstants);
}

void CRenderer::Update_ObjCB(CGameObject* pGameObj)
{
	OBJ_CONSTANT_BUFFER objConstants;
	objConstants.mWorldMat = pGameObj->Get_WorldMatrix().Transpose();
	m_pCurFrameResource->pObjectCB->CopyData(pGameObj->Get_ClonedNum() - 1, objConstants);
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

	// Update PassConstant
	Update_PassCB();
	
	/*XMMATRIX matView = XMMatrixIdentity();
	XMStoreFloat4x4(&passConstants.mViewMat, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&passConstants.mViewInvMat, XMMatrixTranspose(matView));*/

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
					m_pCommandList->SetGraphicsRootSignature(m_pPipelineManager->Get_RootSig(eRootsigType)); // RootSig객체 세팅
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
						m_pCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvUavHandle);

						// ObjCB
						cbvSrvUavHandle.InitOffsetted(pSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0);
						cbvSrvUavHandle.Offset(m_iObjCBVHeapStartOffset);
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

D3D12_GPU_DESCRIPTOR_HANDLE CRenderer::Get_CbvSrvUavStart_GPU()
{
	return m_pGraphic_Device->Get_CbvSrvUavHeapStart_GPU();
}

D3D12_CPU_DESCRIPTOR_HANDLE CRenderer::Get_CbvSrvUavStart_CPU()
{
	return m_pGraphic_Device->Get_CbvSrvUavHeapStart_CPU();
}

void CRenderer::AddTo_RenderGroup(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eRootsigTypeEnum,
                                  CGameObject* pGameObject)
{
	m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eRootsigTypeEnum].push_back(pGameObject);
	Safe_AddRef(pGameObject);
}

void CRenderer::Flush_CommandQueue()
{
	CGraphic_Device::Get_Instance()->Flush_CommandQueue(&m_queue_flush_desc);
}

//--------------------------------------------------------------------------------------

FrameResource::FrameResource(ID3D12Device * pDevice, UINT iObjectCount, UINT iPassCount)
{
	pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&pCmdListAlloc));

	//PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	pObjectCB = CUploadBuffer<OBJ_CONSTANT_BUFFER>::Create(pDevice, iObjectCount, true);
	pPassCB = CUploadBuffer<PASS_CONSTANT_BUFFER>::Create(pDevice, iPassCount, false);
}

FrameResource::~FrameResource()
{
	Safe_Release(pObjectCB);
	Safe_Release(pPassCB);
	Safe_Release(pCmdListAlloc);
}
