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
	m_pCommandAllocator = m_pGraphic_Device->m_pCmdAllocator.Get();
	m_pCommandList = m_pGraphic_Device->m_pCommandList.Get();
	m_pCommandQueue = m_pGraphic_Device->m_pCommandQueue.Get();
	m_pRtvHeap = m_pGraphic_Device->m_pRtvHeap.Get();
	m_pRenderTargetArr = m_pGraphic_Device->m_pRenderTargets->GetAddressOf();
	m_pFence = m_pGraphic_Device->m_pFence.Get();

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

	//	iter.clear(); // 그룹 내 렌더 끝나면 비우기
	//}

	m_pCommandList->SetGraphicsRootSignature(m_pCurRootSig);
	m_pCommandList->SetPipelineState(m_pCurPSO);
	m_pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_pCommandList->IASetVertexBuffers() // In Iter
	m_pCommandList->DrawIndexedInstanced(
		1/*TODO, 그리는 개수만큼 추가*/,
		1,
		0,
		0,
		0);
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
	// Fence
	++m_iFenceValue;
	m_pCommandQueue->Signal(m_pFence, m_iFenceValue);
	// Wait
	const UINT64 iExpectedFenceValue = m_iFenceValue;
	if (m_pFence->GetCompletedValue() < iExpectedFenceValue)
	{
		m_pFence->SetEventOnCompletion(iExpectedFenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
#pragma endregion
}

HRESULT CRenderer::Free()
{	
	for (auto& iter : m_RenderGroup)
	{
		for (auto& innerIter : iter)
		{
			Safe_Release(innerIter);
		}
		iter.clear();
	}

	return CComponent::Free();
}

void CRenderer::AddTo_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pGameObject)
{
	if (eRenderGroup >= RENDERGROUP_END)
	{
		MSG_BOX("Invalid RenderGroup");
	}
	
	m_RenderGroup[eRenderGroup].push_back(pGameObject);

	Safe_AddRef(pGameObject);
}
