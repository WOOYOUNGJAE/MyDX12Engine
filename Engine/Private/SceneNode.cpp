#include "SceneNode.h"
#include "GameObject.h"
#include "SceneNode_AABB.h"
#include "DeviceResource.h"
#include "Device_Utils.h"
#include "DXRRenderer.h"

CSceneNode* CSceneNode::Create(CSceneNode** pChildNodeArr, UINT iArrSize, bool bIsTLAS, CGameObject* pContainingObj)
{
	CSceneNode* pInstance = new CSceneNode;

	pInstance->Set_ContainingObj(pContainingObj);

	if (FAILED(pInstance->Initialize(pChildNodeArr, iArrSize)))
	{
		Safe_Release(pInstance);
		MSG_BOX("SceneNode : Failed to Init");
		return pInstance;
	}

	return pInstance;
}

//static ID3D12Resource* m_bottomLevelAccelerationStructure = nullptr;
HRESULT CSceneNode::Initialize(CSceneNode** pChildNodeArr, UINT iChildArrSize, bool bIsTLAS)
{
	HRESULT hr = S_OK;

	if (iChildArrSize == 2)
	{
		m_pLeftChild = pChildNodeArr[0];
		m_pRightChild = pChildNodeArr[1];
	}
	else // 바이너리 트리가 아닌 경우
	{
		for (UINT i = 0; i < iChildArrSize; ++i)
		{
			m_vecChildNode.emplace_back(pChildNodeArr[i]);
		}
	}

#if DXR_ON
	// DXR을 위한 노드가 아니면 Init 마무리
	if (bIsTLAS == false)
	{
		return hr;
	}

	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
    ID3D12GraphicsCommandList4* pCommandList = CDXRResource::Get_Instance()->Get_CommandList4();
	/*ID3D12Resource* vecUAV_BLAS[1]{};
	vector<ID3D12Resource**> vec_ppUAV_BLAS;
	vec_ppUAV_BLAS.resize(iChildArrSize);
	ID3D12Resource** ppUav_BlasArr[1];*/
	//for (UINT i = 0; i < iChildArrSize; ++i)
	//{
	//	DXR::BLAS*& pBlas = (dynamic_cast<CSceneNode_AABB*>(pChildNodeArr[i])->Get_BLAS());

	//	// Allocate UAV Buffer, 실질적인 BLAS Allocate
	//	hr = MyUtils::AllocateUAVBuffer(pDevice,
	//	pBlas->prebuildInfo.ResultDataMaxSizeInBytes,
	//	//&m_BLAS.uav_BLAS,
	//	&pBlas->uav_BLAS,
	//	D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	//	if (FAILED(hr)) { MSG_BOX("Allocate Failed"); }

	//	pBlas->accelerationStructureDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
	//	pBlas->accelerationStructureDesc.DestAccelerationStructureData = pBlas->uav_BLAS->GetGPUVirtualAddress();

	//	ppUav_BlasArr[0] = &pBlas->uav_BLAS;


	//	UINT iNumPostBuilds = 0;
	//	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* postBuildArr = nullptr;
	//	// 진짜로 GPU에서 BLAS Build
	//	CDXRResource::BuildRaytracingAccelerationStructure(&pBlas->accelerationStructureDesc, iNumPostBuilds, postBuildArr);
	//	CDXRResource::ResourceBarrierUAV(pBlas->uav_BLAS);
	//}
	//::Engine::DXR_Util::Build_TLAS(pDevice, &m_TLAS.uav_TLAS, &m_TLAS.pInstanceDesc, ppUav_BlasArr, iChildArrSize);

#pragma region MyRegion

 //  /* static ID3D12Resource* m_vertexBuffer = nullptr;
 //   static ID3D12Resource* m_indexBuffer = nullptr;*/
 //   /*MyUtils::Create_Buffer_Default(device, commandList, vertices, sizeof(VertexPositionNormalColorTexture) * 3, &intermediateBufferVertex, &m_vertexBuffer);
 //   MyUtils::Create_Buffer_Default(device, commandList, indices, sizeof(UINT16) * 3, &intermediateBufferIndex, &m_indexBuffer);*/
 //   DXR::BLAS*& pBlas = (dynamic_cast<CSceneNode_AABB*>(pChildNodeArr[0])->Get_BLAS());
 //   /*m_vertexBuffer = pBlas->vertexBuffer;
 //   m_indexBuffer = pBlas->indexBuffer;*/


 //   // TLAS
 //   D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
 //   topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
 //   topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
 //   topLevelInputs.NumDescs = 1;
 //   topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

 //   //D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
 //   //device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
 //   //device->GetRaytracingAccelerationStructurePrebuildInfo(&pBlas->accelerationStructureDesc.Inputs, &bottomLevelPrebuildInfo);
 //   D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
 //   device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
 //   //static ID3D12Resource* scratchResource;
	//DXR_Util::AllocateScratch_IfBigger(pDevice, topLevelPrebuildInfo.ScratchDataSizeInBytes);
 //   //ID3D12Resource* m_topLevelAccelerationStructure = nullptr;
 //   //MyUtils::AllocateUAVBuffer(device, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, pBlas->prebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
 //   // Real Allocate
 //   //MyUtils::AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"BottomLevelAccelerationStructure");
 //   MyUtils::AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_TLAS.uav_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

 //   ID3D12Resource* instanceDescs;
 //   D3D12_RAYTRACING_INSTANCE_DESC* instanceDescArr = new D3D12_RAYTRACING_INSTANCE_DESC[1]{};
 //   for (UINT i = 0; i < 1; ++i)
 //   {
 //       instanceDescArr[i].Transform[0][0] = instanceDescArr[i].Transform[1][1] = instanceDescArr[i].Transform[2][2] = 1;
 //       instanceDescArr[i].InstanceMask = 1;
 //       //instanceDescArr[i].AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
 //       instanceDescArr[i].AccelerationStructure = pBlas->uav_BLAS->GetGPUVirtualAddress();
 //   }
 //   MyUtils::AllocateUploadBuffer(device, instanceDescArr, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * 1, &instanceDescs);


 //   // Bottom Level Acceleration Structure desc
 //  /* D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
 //   {
 //       bottomLevelBuildDesc.Inputs = pBlas->accelerationStructureDesc.Inputs;
 //       bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
 //       bottomLevelBuildDesc.DestAccelerationStructureData = pBlas->uav_BLAS->GetGPUVirtualAddress();
 //   }*/

 //   // Top Level Acceleration Structure desc
 //   D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
 //   {
 //       topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
 //       topLevelBuildDesc.Inputs = topLevelInputs;
 //       topLevelBuildDesc.DestAccelerationStructureData = m_TLAS.uav_TLAS->GetGPUVirtualAddress();
 //       topLevelBuildDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
 //   }

	// 
	DXR::BLAS** pBlasArr = new DXR::BLAS*[iChildArrSize];
	for (UINT i = 0; i < iChildArrSize; ++i)
	{
		pBlasArr[i] = dynamic_cast<CSceneNode_AABB*>(pChildNodeArr[0])->Get_BLAS();
	}
	DXR_Util::Build_TLAS0(pDevice, pCommandList, &m_TLAS.uav_TLAS, &m_TLAS.pInstanceDesc, pBlasArr, iChildArrSize);
	Safe_Delete_Array(pBlasArr);
 //   // Build acceleration structure.
 //   //commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
 //   /*pCommandList->BuildRaytracingAccelerationStructure(&pBlas->accelerationStructureDesc, 0, nullptr);
 //   pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(pBlas->uav_BLAS));*/
 //   //pCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    //pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_TLAS.uav_TLAS));

	

#pragma endregion

#endif

	return hr;
}

HRESULT CSceneNode::Free()
{
#if DXR_ON
	Safe_Release(m_TLAS.uav_TLAS);
	Safe_Release(m_TLAS.pInstanceDesc);
#endif
	return S_OK;
}

void CSceneNode::Set_ContainingObj(CGameObject* pGameObj)
{
	m_pContainingObj = pGameObj;
}
