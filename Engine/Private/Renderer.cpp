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
	m_pGraphic_Device = CGraphic_Device::Get_Instance();
	m_pPipelineManager = CPipelineManager::Get_Instance();
	/*m_pCommandAllocator = m_pGraphic_Device->m_pCmdAllocator.Get();
	m_pCommandList = m_pGraphic_Device->m_pCommandList.Get();
	m_pCommandQueue = m_pGraphic_Device->m_pCommandQueue.Get();*/
	m_pRtvHeap = m_pGraphic_Device->m_pRtvHeap.Get();
	m_pRenderTargetArr = m_pGraphic_Device->m_pRenderTargets->GetAddressOf();
	//m_pFence = m_pGraphic_Device->m_pFence.Get();

	ID3D12Device* pDevice = m_pGraphic_Device->m_pDevice.Get();

	// Init Fence
	if (FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence))))
	{
		MSG_BOX("Failed To Create Fence");
		return E_FAIL;
	}
	// Fence Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);


	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
	{
		return E_FAIL;
	}

	if (FAILED(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pCommandAllocator))))
	{
		return E_FAIL;
	}

	if (FAILED(pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT, // 나중에 Bundle 사용할 수도
		m_pCommandAllocator, // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(&m_pCommandList))))
	{
		return E_FAIL;
	}

	// 닫힌 상태로 시작
	m_pCommandList->Close();

	m_queue_flush_queue = {
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

void CRenderer::BeginRender()
{
	m_pCommandAllocator->Reset();
	m_pCommandList->Reset(m_pCommandAllocator, nullptr);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
	
	m_pCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargetArr[m_iFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->ClearRenderTargetView(m_pGraphic_Device->CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);

	m_pCommandList->RSSetViewports(1, &m_pGraphic_Device->m_screenViewport);
	m_pCommandList->RSSetScissorRects(1, &m_pGraphic_Device->m_ScissorRect);
	m_pCommandList->OMSetRenderTargets(1, &rtvHeapHandle, FALSE, nullptr);
}

void CRenderer::MainRender()
{
	//for (auto& iter : m_RenderGroup)
	//{
	//	for (auto& innerIter : iter)
	//	{
	//		innerIter->Render();
	//		Safe_Release(innerIter);
	//	}
	//	
	//	iter.clear(); // 그룹 내 렌더 끝나면 비우기
	//}

	for (UINT IsFirst = 0; IsFirst < RENDER_PRIORITY_END; ++IsFirst)
	{
		for (UINT eBlendModeEnum = 0; eBlendModeEnum < RENDER_BLENDMODE_END; ++eBlendModeEnum)
		{
			for (UINT eRootsigEnum = 0; eRootsigEnum < RENDER_ROOTSIGTYPE_END; ++eRootsigEnum)
			{
				m_pCommandList->SetGraphicsRootSignature(m_pPipelineManager->Get_RootSig(eRootsigEnum));
				for (UINT eShaderTypeEnum = 0; eShaderTypeEnum < RENDER_SHADERTYPE_END; ++eShaderTypeEnum)
				{
					ID3D12PipelineState* pPSO = m_pPipelineManager->Get_PSO(IsFirst, eBlendModeEnum, eRootsigEnum, eShaderTypeEnum);
					if (pPSO == nullptr)
					{
						continue;
					}
					m_pCommandList->SetPipelineState(pPSO);

					for (auto& iter : m_RenderGroup[IsFirst][eBlendModeEnum][eRootsigEnum][eShaderTypeEnum])
					{
						m_pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						//m_pCommandList->IASetVertexBuffers() // In Iter
						m_pCommandList->DrawIndexedInstanced(
							1/*TODO, 그리는 개수만큼 추가*/,
							1,
							0,
							0,
							0);
						//Safe_Release(iter);
					}
					m_RenderGroup[IsFirst][eBlendModeEnum][eRootsigEnum][eShaderTypeEnum].clear();
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
	CGraphic_Device::Get_Instance()->Flush_CommandQueue(&m_queue_flush_queue);
#pragma endregion
}

HRESULT CRenderer::Free()
{
	CloseHandle(m_fenceEvent);

	Safe_Release(m_pCommandList);
	Safe_Release(m_pCommandAllocator);
	Safe_Release(m_pCommandQueue);
	Safe_Release(m_pFence);

	/*for (auto& iter : m_RenderGroup)
	{
		for (auto& innerIter : iter)
		{
			Safe_Release(innerIter);
		}
		iter.clear();
	}*/

	return CComponent::Free();
}

void CRenderer::AddTo_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pGameObject)
{
	if (eRenderGroup >= RENDERGROUP_END)
	{
		MSG_BOX("Invalid RenderGroup");
	}
	
	//m_RenderGroup[eRenderGroup].push_back(pGameObject);

	Safe_AddRef(pGameObject);
}

void CRenderer::AddTo_RenderGroup(UINT IsFirst, UINT eBlendModeEnum, UINT eRootsigEnum, UINT eShaderTypeEnum,
	CGameObject* pGameObject)
{
	m_RenderGroup[IsFirst][eBlendModeEnum][eRootsigEnum][eShaderTypeEnum].push_back(pGameObject);
}
