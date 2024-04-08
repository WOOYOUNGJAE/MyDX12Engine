#include "DXRRenderer.h"
#include "DXRResource.h"
#include "DeviceResource.h"
#include "MeshData.h"
#include "DeviceResource.h"
#include "AssetManager.h"
#include "FrameResource.h"
#include "BVH.h"
#include "SceneNode_AABB.h"
#include "SDSManager.h"

#if DXR_ON
CDXRRenderer* CDXRRenderer::Create()
{
	CDXRRenderer* pInstance = new CDXRRenderer;

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("DXR: Create Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

HRESULT CDXRRenderer::Initialize()
{
	HRESULT hr = S_OK;	

    // set pointers
    m_pDXRResources = CDXRResource::Get_Instance();
    m_pDeviceResource = CDeviceResource::Get_Instance();
    m_pCommandAllocatorArr = m_pDXRResources->m_pCommandAllocatorArr;
    m_pCommandList = m_pDXRResources->m_pCommandList;
    m_pDXR_PSO = m_pDXRResources->m_pDXR_PSO;
    m_pRenderTargetArr = CDeviceResource::Get_Instance()->m_pRenderTargets->GetAddressOf();

    // Since each shader table has only one shader record, the stride is same as the size. 임시로 레코드 1개일 떄
    ZeroMemory(&m_disptchRaysDesc, sizeof(D3D12_DISPATCH_RAYS_DESC));
    m_disptchRaysDesc.RayGenerationShaderRecord.StartAddress = m_pDXRResources->m_pRayGenShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.RayGenerationShaderRecord.SizeInBytes = m_pDXRResources->m_pRayGenShaderTable->GetDesc().Width;
    m_disptchRaysDesc.MissShaderTable.StartAddress = m_pDXRResources->m_pMissShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.MissShaderTable.SizeInBytes = m_pDXRResources->m_pMissShaderTable->GetDesc().Width;
    m_disptchRaysDesc.MissShaderTable.StrideInBytes = m_disptchRaysDesc.MissShaderTable.SizeInBytes; //
    m_disptchRaysDesc.HitGroupTable.StartAddress = m_pDXRResources->m_pHitGroupShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.HitGroupTable.SizeInBytes = m_pDXRResources->m_pHitGroupShaderTable->GetDesc().Width;
    m_disptchRaysDesc.HitGroupTable.StrideInBytes = m_disptchRaysDesc.HitGroupTable.SizeInBytes; // 
    m_disptchRaysDesc.Width = m_pDXRResources->m_iScreenWidth;
    m_disptchRaysDesc.Height = m_pDXRResources->m_iScreenHeight;
    m_disptchRaysDesc.Depth = 1;

	return hr;
}

HRESULT CDXRRenderer::Free()
{
	return S_OK;
}


void CDXRRenderer::BeginRender()
{
    m_pCommandAllocatorArr[m_pDeviceResource->m_iCurrBackBuffer]->Reset();
    m_pCommandList->Reset(m_pCommandAllocatorArr[m_pDeviceResource->m_iCurrBackBuffer], nullptr);
    //
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pDeviceResource->m_pRenderTargets[m_pDeviceResource->m_iCurrBackBuffer].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    m_pCommandList->ResourceBarrier(1, &barrier);

}

void CDXRRenderer::MainRender()
{
    Set_ComputeRootDescriptorTable_Global();

    DispatchRay();
}

void CDXRRenderer::EndRender()
{

#pragma region Copy DXR Output to Back Buffer
    D3D12_RESOURCE_BARRIER preCopyBarriers[2];
    // 렌더 타겟 쓸 수 있도록
    preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargetArr[m_iFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    // DXR 결과물 Copy Src로
    preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_pDXRResources->m_pDXROutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    m_pCommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    m_pCommandList->CopyResource(m_pRenderTargetArr[m_iFrameIndex], m_pDXRResources->m_pDXROutput);

    D3D12_RESOURCE_BARRIER postCopyBarriers[2]; // 위의 역순
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargetArr[m_iFrameIndex], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_pDXRResources->m_pDXROutput, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    m_pCommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
#pragma endregion

    m_pCommandList->Close(); // 기록 중단
    m_pDXRResources->Execute_CommnadList();
} 

void CDXRRenderer::Present()
{
    // Swap the back and front buffers
    HRESULT hr = m_pDeviceResource->m_pSwapChain->Present(0, 0);
    if (FAILED(hr)) { MSG_BOX("Present Failed"); }

    m_iFrameIndex = (m_pDeviceResource->m_iCurrBackBuffer + 1) % m_pDeviceResource->m_iSwapChainBufferCount;
    m_pDeviceResource->m_iCurrBackBuffer = (m_pDeviceResource->m_iCurrBackBuffer + 1) % m_pDeviceResource->m_iSwapChainBufferCount;
    m_pDXRResources->Flush_CommandQueue();
}

void CDXRRenderer::Do_RayTracing()
{

}

void CDXRRenderer::DispatchRay()
{
    m_pCommandList->SetPipelineState1(m_pDXR_PSO);
    m_pCommandList->DispatchRays(&m_disptchRaysDesc);
}

void CDXRRenderer::Set_ComputeRootDescriptorTable_Global()
{
    std::vector<CBVH*> refVecAccelerationTree = CSDSManager::Get_Instance()->Get_vecAccelerationTree();

    D3D12_GPU_VIRTUAL_ADDRESS TLAS_GPU_Adress = refVecAccelerationTree[0]->Get_Root()->Get_TLAS().uav_TLAS->GetGPUVirtualAddress();


    m_pCommandList->SetDescriptorHeaps(1, &m_pDXRResources->m_pDescriptorHeap);
    m_pCommandList->SetComputeRootSignature(m_pDXRResources->m_pRootSigArr[DXR_ROOTSIG_GLOBAL]);
    m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::RENDER_TARGET, m_pDXRResources->m_DXROutputHeapHandle);
    m_pCommandList->SetComputeRootShaderResourceView(GlobalRootSigSlot::AS, TLAS_GPU_Adress); // 빌드할 때는 UAV였는데??
}

#endif
