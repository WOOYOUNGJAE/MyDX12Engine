#include "DXRRenderer.h"
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
ID3D12Resource* m_vertexBuffer;
ID3D12Resource* m_indexBuffer;
ID3D12Resource* CDXRRenderer::m_bottomLevelAccelerationStructure;
ID3D12Resource* m_topLevelAccelerationStructure;

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
    m_disptchRaysDesc.HitGroupTable.StartAddress = m_pDXRResources->m_pHitGroupShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.HitGroupTable.SizeInBytes = m_pDXRResources->m_pHitGroupShaderTable->GetDesc().Width;
    m_disptchRaysDesc.HitGroupTable.StrideInBytes = m_disptchRaysDesc.HitGroupTable.SizeInBytes; // 
    m_disptchRaysDesc.MissShaderTable.StartAddress = m_pDXRResources->m_pMissShaderTable->GetGPUVirtualAddress();
    m_disptchRaysDesc.MissShaderTable.SizeInBytes = m_pDXRResources->m_pMissShaderTable->GetDesc().Width;
    m_disptchRaysDesc.MissShaderTable.StrideInBytes = m_disptchRaysDesc.MissShaderTable.SizeInBytes; //
    m_disptchRaysDesc.Width = m_pDXRResources->m_iScreenWidth;
    m_disptchRaysDesc.Height = m_pDXRResources->m_iScreenHeight;
    m_disptchRaysDesc.Depth = 1;

//
//#pragma region TempTest
//    auto device = m_pDeviceResource->Get_Device5();
//    UINT16 indices[] =
//    {
//        0, 1, 2
//    };
//
//    float depthValue = 1.0;
//    float offset = 0.7f;
//
//    VertexPositionNormalColorTexture vertices[]
//    { {Vector3(0.f, 100.f, 1.5f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.5f, 0.0f)},
//        {Vector3(100.f, -100.f, 1.5f), Vector3(0.0f, 0.0f, -1.0f),Vector2(1.0f, 1.0f)},
//        {Vector3(-100.f, -100.f, 1.5f), Vector3(0.0f, 0.0f, -1.0f),Vector2(0.0f, 1.0f)}
//    };
//
//    //VertexPosition vertices[]
//    //{
//    //    // The sample raytraces in screen space coordinates.
//    //    // Since DirectX screen space coordinates are right handed (i.e. Y axis points down).
//    //    // Define the vertices in counter clockwise order ~ clockwise in left handed.
//    //    Vector3{  0, -offset, depthValue },
//    //    Vector3{ -offset, offset, depthValue },
//    //    Vector3{ offset, offset, depthValue }
//    //};
//
//    /*MyUtils::AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer);
//    MyUtils::AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer);*/
//    //--
//    auto commandList = m_pCommandList;
//    auto commandQueue = m_pDXRResources->m_pCommandQueue;
//    auto commandAllocator = m_pCommandAllocatorArr[0];
//
//    // Reset the command list for the acceleration structure construction.
//    commandList->Reset(commandAllocator, nullptr);
//
//    ID3D12Resource* intermediateBufferVertex = nullptr;
//    ID3D12Resource* intermediateBufferIndex = nullptr;
//    MyUtils::Create_Buffer_Default(device, commandList, vertices, sizeof(VertexPositionNormalColorTexture) * 3, &intermediateBufferVertex, &m_vertexBuffer);
//    MyUtils::Create_Buffer_Default(device, commandList, indices, sizeof(UINT16) * 3, &intermediateBufferIndex, &m_indexBuffer);
//
//
//    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
//    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
//    geometryDesc.Triangles.IndexBuffer = m_indexBuffer->GetGPUVirtualAddress();
//    geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexBuffer->GetDesc().Width) / sizeof(UINT16);
//    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
//    geometryDesc.Triangles.Transform3x4 = 0;
//    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
//    geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer->GetDesc().Width) / sizeof(VertexPositionNormalColorTexture);
//    geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer->GetGPUVirtualAddress();
//    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(VertexPositionNormalColorTexture);
//    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
//
//    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs{};
//    bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//    bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
//    bottomLevelInputs.NumDescs = 1;
//    bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
//    bottomLevelInputs.pGeometryDescs = &geometryDesc;
//
//    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
//    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//    topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
//    topLevelInputs.NumDescs = 1;
//    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
//
//    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
//    device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
//    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
//    device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
//    ComPtr<ID3D12Resource> scratchResource;
//	MyUtils::AllocateUAVBuffer(device, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
//    {
//        MyUtils::AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"BottomLevelAccelerationStructure");
//        MyUtils::AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");
//    }
//    
//    ComPtr<ID3D12Resource> instanceDescs;
//    D3D12_RAYTRACING_INSTANCE_DESC* instanceDescArr = new D3D12_RAYTRACING_INSTANCE_DESC[1]{};
//    for (UINT i = 0; i < 1; ++i)
//    {
//        instanceDescArr[i].Transform[0][0] = instanceDescArr[i].Transform[1][1] = instanceDescArr[i].Transform[2][2] = 1;
//        instanceDescArr[i].InstanceMask = 1;
//        instanceDescArr[i].AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
//    }
//    MyUtils::AllocateUploadBuffer(device, instanceDescArr, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * 1, &instanceDescs);
//
//
//    // Bottom Level Acceleration Structure desc
//    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
//    {
//        bottomLevelBuildDesc.Inputs = bottomLevelInputs;
//        bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
//        bottomLevelBuildDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
//    }
//
//    // Top Level Acceleration Structure desc
//    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
//    {
//        topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
//        topLevelBuildDesc.Inputs = topLevelInputs;
//        topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
//        topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
//    }
//
//    auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
//    {
//        raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
//        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure));
//        raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
//        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_topLevelAccelerationStructure));
//    };
//
//    // Build acceleration structure.
//    BuildAccelerationStructure(m_pCommandList);
//
//    // Kick off acceleration structure construction.
//    m_pDXRResources->Close_CommandList();
//    m_pDXRResources->Execute_CommnadList();
//    m_pDXRResources->Flush_CommandQueue();
//
//#pragma endregion TempTest



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
    curSceneCB.lightPosition = Vector4(0.0f, 18.0f, -20.0f, 0.0f);
    curSceneCB.lightAmbientColor = Vector4(0.25f, 0.25f, 0.25f, 1.0f);
    curSceneCB.lightDiffuseColor = Vector4(0.6f, 0.6f, 0.6f, 1.f);
}

void CDXRRenderer::Update_Dynamic_PassCB()
{
    CCamera* pMainCam = CCameraManager::Get_Instance()->Get_MainCam();

    Vector3 vCamPos = pMainCam->Get_Pos();
    curSceneCB.cameraPosition = Vector4(vCamPos.x, vCamPos.y, vCamPos.z, 1);

    Matrix viewMat = pMainCam->Get_WorldMatrix().Invert();
    curSceneCB.projectionToWorld = XMMatrixInverse(nullptr, viewMat * m_mProj);

    m_pCurFrameResource->pPassCB_DXR_scene->CopyData(0, curSceneCB);
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
    Update_Dynamic_PassCB();

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

void CDXRRenderer::DispatchRay()
{
    m_pCommandList->SetPipelineState1(m_pDXR_PSO);
    m_pCommandList->DispatchRays(&m_disptchRaysDesc);
}

void CDXRRenderer::Set_ComputeRootDescriptorTable_Global()
{
    std::vector<CBVH*> refVecAccelerationTree = CSDSManager::Get_Instance()->Get_vecAccelerationTree();

    D3D12_GPU_VIRTUAL_ADDRESS TLAS_GPU_Adress = refVecAccelerationTree[0]->Get_Root()->Get_TLAS().uav_TLAS->GetGPUVirtualAddress();

    CD3DX12_GPU_DESCRIPTOR_HANDLE IB_VB_SRV_Handle_GPU =
        m_pDXRResources->Get_HeapHandleGPU(refVecAccelerationTree[0]->Get_Root()->Get_TLAS().IB_VB_SRV_startOffsetInDescriptors);

    m_pCommandList->SetDescriptorHeaps(1, &m_pDXRResources->m_pDescriptorHeap);
    m_pCommandList->SetComputeRootSignature(m_pDXRResources->m_pRootSigArr[DXR_ROOTSIG_GLOBAL]);
    m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::RENDER_TARGET, m_pDXRResources->m_DXROutputHeapHandle);
    m_pCommandList->SetComputeRootShaderResourceView(GlobalRootSigSlot::AS, TLAS_GPU_Adress); // 빌드할 때는 UAV였는데??

    m_pCommandList->SetComputeRootDescriptorTable(GlobalRootSigSlot::IB_VB_SRV, IB_VB_SRV_Handle_GPU);

    m_pCommandList->SetComputeRootConstantBufferView(GlobalRootSigSlot::PASS_CONSTANT, m_pCurFrameResource->pPassCB_vp_light->Get_UploadBuffer()->GetGPUVirtualAddress());
}

#endif
