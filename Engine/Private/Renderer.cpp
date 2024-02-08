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
	m_pCommandList = m_pGraphic_Device->m_pCommandList.Get();*/
	m_pCommandQueue = m_pGraphic_Device->m_pCommandQueue.Get(); // CommandQueue는 따로 만들지 않고 공유, 스왑체인이 매치되어야 하기 때문
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


	/*D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
	{
		return E_FAIL;
	}*/

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
	for (UINT IsFirst = 0; IsFirst < RENDER_PRIORITY_END; ++IsFirst)
	{
		for (UINT eBlendModeEnum = 0; eBlendModeEnum < RENDER_BLENDMODE_END; ++eBlendModeEnum)
		{
			for (UINT eShaderTypeEnum = 0; eShaderTypeEnum < RENDER_SHADERTYPE_END; ++eShaderTypeEnum)
			{
				for (UINT eParamComboType = 0; eParamComboType < RENDER_PARAMCOMBO_END; ++eParamComboType)
				{
					if (m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eParamComboType].empty())
					{
						continue;
					}

					ID3D12PipelineState* pPSO = m_pPipelineManager->Get_PSO(IsFirst, eBlendModeEnum, eParamComboType, eShaderTypeEnum);
					if (pPSO == nullptr)
					{
						continue;
					}
					m_pCommandList->SetGraphicsRootSignature(m_pPipelineManager->Get_RootSig(eParamComboType)); // RoogSig객체 세팅

					if (eParamComboType != PARAM_SIMPLE)
					{
						ID3D12DescriptorHeap* pSrvHeap = m_pGraphic_Device->Get_CbvSrvUavHeap();
						ID3D12DescriptorHeap* ppHeaps[] = { pSrvHeap };
						m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
						m_pCommandList->SetGraphicsRootDescriptorTable(0, pSrvHeap->GetGPUDescriptorHandleForHeapStart()); // 세팅된 RootSig의 어디?
					}

					m_pCommandList->SetPipelineState(pPSO);

					for (auto& iter : m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eParamComboType])
					{
						m_pCommandList->IASetPrimitiveTopology(iter->PrimitiveType());
						m_pCommandList->IASetVertexBuffers(0, 1, &iter->VertexBufferView());
						//m_pCommandList->IASetIndexBuffer(&iter->IndexBufferView());

						m_pCommandList->DrawInstanced(3, 1, 0, 0);
						/*m_pCommandList->DrawIndexedInstanced(
							3,
							1,
							0,
							0,
							0);*/
						Safe_Release(iter);
					}
					m_RenderGroup[IsFirst][eBlendModeEnum][eShaderTypeEnum][eParamComboType].clear();
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

void CRenderer::AddTo_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pGameObject)
{
	if (eRenderGroup >= RENDERGROUP_END)
	{
		MSG_BOX("Invalid RenderGroup");
	}
	
	//m_RenderGroup[eRenderGroup].push_back(pGameObject);

	Safe_AddRef(pGameObject);
}

void CRenderer::AddTo_RenderGroup(UINT IsFirst, UINT eBlendModeEnum, UINT eShaderTypeEnum, UINT eParamComboEnum,
                                  CGameObject* pGameObject)
{
	m_RenderGroup[IsFirst][eBlendModeEnum][eParamComboEnum][eShaderTypeEnum].push_back(pGameObject);
	Safe_AddRef(pGameObject);
}
