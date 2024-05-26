#include "DXRRenderer.h"

#include "Asset.h"
#include "DXRResource.h"
#include "DeviceResource.h"
#include "MeshData.h"
#include "DeviceResource.h"
#include "AssetManager.h"
#include "FrameResource.h"
#include "BVH.h"
#include "CameraManager.h"
#include "SceneNode_AABB.h"
#include "SDSManager.h"
#include "Camera.h"
#include "FrameResourceManager.h"
#include "GameObjectManager.h"
#include "Material.h"
#include "ObjLayer.h"

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
    m_pAllObjLayers = CGameObjectManager::Get_Instance()->Get_AllObjLayers();

    // Since each shader table has only one shader record, the stride is same as the size. 임시로 레코드 1개일 떄
    ZeroMemory(&m_disptchRaysDesc, sizeof(D3D12_DISPATCH_RAYS_DESC));
    m_disptchRaysDesc.RayGenerationShaderRecord.StartAddress = m_pDXRResources->m_pRayGenShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.RayGenerationShaderRecord.SizeInBytes = m_pDXRResources->m_pRayGenShaderTable->GetDesc().Width;
    m_disptchRaysDesc.HitGroupTable.StartAddress = m_pDXRResources->m_pHitGroupShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.HitGroupTable.SizeInBytes = m_pDXRResources->m_pHitGroupShaderTable->GetDesc().Width;
    m_disptchRaysDesc.HitGroupTable.StrideInBytes = m_pDXRResources->m_iHitGroupRecordSizeInBytes;
    m_disptchRaysDesc.MissShaderTable.StartAddress = m_pDXRResources->m_pMissShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.MissShaderTable.SizeInBytes = m_pDXRResources->m_pMissShaderTable->GetDesc().Width;
    m_disptchRaysDesc.MissShaderTable.StrideInBytes = m_pDXRResources->m_iMissRecordSizeInBytes;
    m_disptchRaysDesc.Width = m_pDXRResources->m_iScreenWidth;
    m_disptchRaysDesc.Height = m_pDXRResources->m_iScreenHeight;
    m_disptchRaysDesc.Depth = 1;

	return hr;
}

HRESULT CDXRRenderer::Free()
{
	return S_OK;
}

void CDXRRenderer::Set_FrameResource()
{
    m_pCurFrameResource = CFrameResourceManager::Get_Instance()->m_vecFrameResource[0]; // TODO : FrameResource 여러개 변경 예정
}

void CDXRRenderer::Update_Static_PassCB(const CAMERA_DESC& camDesc)
{
    m_mProj = XMMatrixPerspectiveFovLH(camDesc.fFovy, camDesc.fAspectRatio, camDesc.fNear, camDesc.fFar);

    // Light Info
    curSceneCB.lightPosition = Vector4(20.0f, 20.0f, -20.0f, 0.0f);
    curSceneCB.lightAmbientColor = Vector4::One * 0.25f;
    curSceneCB.lightDiffuseColor = Vector4::One;
    //curSceneCB.lightDiffuseColor = Vector4(0.6f, 0.6f, 0.6f, 1.f);
}

void CDXRRenderer::Update_Dynamic_PassCB()
{
    CCamera* pMainCam = CCameraManager::Get_Instance()->Get_MainCam();

    Matrix vCamWorldMat = pMainCam->Get_WorldMatrix();
    Vector3 vCamPos = pMainCam->Get_Pos();
    curSceneCB.cameraPosition = Vector4(vCamPos.x, vCamPos.y, vCamPos.z, 1);

    Matrix viewMat = pMainCam->Get_WorldMatrix().Invert();

    curSceneCB.viewProjectionInv = XMMatrixTranspose(XMMatrixInverse(nullptr, viewMat * m_mProj));

    m_pCurFrameResource->pPassCB_DXR_scene->CopyData(0, curSceneCB);
}

void CDXRRenderer::Update_Static_Object_CB()
{
    CMeshData** pSingleMeshDataArr = CAssetManager::Get_Instance()->Get_SingleMeshDataArr();

    for (auto& pair : *m_pAllObjLayers)
    {
        // pair.second == 오브젝트 리스트 관리하는 레이어
	    for(auto& iterGameObject : pair.second->Get_ObjList())
	    {
            UINT iRenderNumbering_ZeroIfNotRendered = iterGameObject->Get_RenderNumbering();
		    if (iRenderNumbering_ZeroIfNotRendered > 0)
		    {
                CMaterial* pMaterial = iterGameObject->Get_Material();
                if (pMaterial == nullptr)
                {
                    // 렌더링되는 오브젝트가 아닌 경우
                    continue;
                }
                DXR::OBJECT_CB_STATIC objectCB_Static = DXR::OBJECT_CB_STATIC(pMaterial->Get_DXR_MaterialInfo());
                objectCB_Static.startIndex_in_IB_SRV = pSingleMeshDataArr[iterGameObject->Get_GeometryType()]->Get_refBLAS().iStartIndex_in_IB_SRV;
                objectCB_Static.startIndex_in_VB_SRV = pSingleMeshDataArr[iterGameObject->Get_GeometryType()]->Get_refBLAS().iStartIndex_in_VB_SRV;
                

                m_pCurFrameResource->pObjectCB_Static_DXR->CopyData(iRenderNumbering_ZeroIfNotRendered - 1, objectCB_Static);
		    }
	    }
    }

    /*DXR_Util::Update_ShaderRecord(m_pCommandList,
        m_pCurFrameResource->pObjectCB_Static_DXR->Get_UploadBuffer(),
        m_pDXRResources->m_pHitGroupShaderTable,
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
        NUM_OBJECTS, sizeof(DXR::OBJECT_CB_STATIC));*/


    DXR_Util::Update_ShaderRecord(m_pCommandList,
                                  m_pCurFrameResource->pObjectCB_Static_DXR->Get_UploadBuffer(),
                                  m_pDXRResources->m_pHitGroupShaderTable,
                                  D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
                                  NUM_OBJECTS,
                                  sizeof(DXR::OBJECT_CB_STATIC),
                                  0/*STATIC CB is First*/);
}

void CDXRRenderer::Update_Dynamic_Object_CB()
{
    for (auto& pair : *m_pAllObjLayers)
    {
        // pair.second == 오브젝트 리스트 관리하는 레이어
        for (auto& iterGameObject : pair.second->Get_ObjList())
        {
            UINT iRenderNumbering_ZeroIfNotRendered = iterGameObject->Get_RenderNumbering();
            if (iRenderNumbering_ZeroIfNotRendered > 0)
            {
                Matrix worldMatrix = iterGameObject->Get_WorldMatrix();
                DXR::OBJECT_CB_DYNAMIC objectCB_Dynamic{};
                // 역행렬의 전치행렬
                objectCB_Dynamic.InvTranspose = worldMatrix.Invert().Transpose();
                //objectCB_Dynamic.InvTranspose = worldMatrix.Invert();

                m_pCurFrameResource->pObjectCB_Dynamic_DXR->CopyData(iRenderNumbering_ZeroIfNotRendered - 1, objectCB_Dynamic);
            }
        }
    }

    DXR_Util::Update_ShaderRecord(m_pCommandList,
        m_pCurFrameResource->pObjectCB_Dynamic_DXR->Get_UploadBuffer(),
        m_pDXRResources->m_pHitGroupShaderTable,
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
        NUM_OBJECTS,
        sizeof(DXR::OBJECT_CB_DYNAMIC),
        sizeof(DXR::OBJECT_CB_STATIC) * NUM_OBJECTS/*STATIC CB is First*/);
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
    // start to Create new Commands
    m_pCommandList->Reset(m_pCommandAllocatorArr[m_pDeviceResource->m_iCurrBackBuffer], nullptr);

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
    m_pDXRResources->Execute_CommandList();
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

void CDXRRenderer::DispatchRay()
{
    m_pCommandList->SetPipelineState1(m_pDXR_PSO);
    m_pCommandList->DispatchRays(&m_disptchRaysDesc);
}

void CDXRRenderer::Set_ComputeRootDescriptorTable_Global()
{
    std::vector<CBVH*> refVecAccelerationTree = CSDSManager::Get_Instance()->Get_vecAccelerationTree();

    D3D12_GPU_VIRTUAL_ADDRESS TLAS_GPU_Adress = refVecAccelerationTree[SDS_AS]->Get_Root()->Get_TLAS().uav_TLAS->GetGPUVirtualAddress();

    CD3DX12_GPU_DESCRIPTOR_HANDLE IB_VB_SRV_Handle_GPU =
        m_pDXRResources->Get_HeapHandleGPU(CAssetManager::Get_Instance()->Get_IB_VB_SRV_startOffset());
        //m_pDXRResources->Get_HeapHandleGPU(refVecAccelerationTree[SDS_AS]->Get_Root()->Get_TLAS().IB_VB_SRV_startOffsetInDescriptors);

    m_pCommandList->SetDescriptorHeaps(1, &m_pDXRResources->m_pDescriptorHeap);
    m_pCommandList->SetComputeRootSignature(m_pDXRResources->m_pRootSigArr[DXR_ROOTSIG_GLOBAL]);
    m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::RENDER_TARGET, m_pDXRResources->m_DXROutputHeapHandle);
    m_pCommandList->SetComputeRootShaderResourceView(GlobalRootSigSlot::AS, TLAS_GPU_Adress); // 빌드할 때는 UAV였는데??

    m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::IB_VB_SRV, IB_VB_SRV_Handle_GPU);

    m_pCommandList->SetComputeRootConstantBufferView(GlobalRootSigSlot::PASS_CONSTANT, m_pCurFrameResource->pPassCB_DXR_scene->Get_UploadBuffer()->GetGPUVirtualAddress());
}

#endif
