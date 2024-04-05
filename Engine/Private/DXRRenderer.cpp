#include "DXRRenderer.h"
#include "DXRResource.h"
#include "DeviceResource.h"
#include "MeshData.h"
#include "DeviceResource.h"
#include "AssetManager.h"
#include "FrameResource.h"
#include "./Shaders/Raytracing.hlsl.h"

#if DXR_ON
CDXRRenderer* CDXRRenderer::Create()
{
	CDXRRenderer* pInstance = new CDXRRenderer;

	/*if (FAILED(pInstance->Initialize(ppDevice)))
	{
		MSG_BOX("DXR: Create Failed");
		Safe_Release(pInstance);
	}*/

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
    static bool initComplete = false;
if (initComplete == false)
{




    initComplete = true;
}
}

void CDXRRenderer::EndRender()
{
} 

void CDXRRenderer::Present()
{
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
    m_pCommandList->SetDescriptorHeaps(1, &m_pDXRResources->m_pDescriptorHeap);
    // indexBuffer과 VertexBuffer가 연속적이라 index만 세팅
    //m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::IB_VB_SRV, m_indexBuffer.gpuDescriptorHandle);
    //m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::RENDER_TARGET, m_raytracingOutputResourceUAVGpuDescriptor);
}

#endif
