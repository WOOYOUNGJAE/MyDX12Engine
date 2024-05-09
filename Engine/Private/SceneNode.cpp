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
	CDXRResource* pDXRResource = CDXRResource::Get_Instance();

	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
    ID3D12GraphicsCommandList4* pCommandList = pDXRResource->Get_CommandList4();
	DXR::BLAS** pBlasArr = new DXR::BLAS*[iChildArrSize];
	for (UINT i = 0; i < iChildArrSize; ++i)
	{
		pBlasArr[i] = dynamic_cast<CSceneNode_AABB*>(pChildNodeArr[0])->Get_BLAS();
	}
	

	m_TLAS.IB_VB_SRV_startOffsetInDescriptors = MyUtils::Get_HeapStartOffset(
		pDXRResource->Get_HeapHandleStart_CPU(),
		pDXRResource->Get_refHeapHandle_CPU(),
		pDXRResource->Get_DescriptorSize()
	);

	//----------------------
	ID3D12Resource** ppOutUAV_TLAS = &m_TLAS.uav_TLAS;
	ID3D12Resource** ppOutInstanceDescResource = &m_TLAS.pInstanceDesc;
	DXR::BLAS** pBlassArr = pBlasArr;
	UINT iNumBlas = 1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = CDXRResource::Get_Instance()->Get_refHeapHandle_CPU();
	UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#pragma region Create Serial SRV of IB
	//for (UINT i = 0; i < iNumBlas; ++i)
	//{
	//	UINT iSingleIdexElementSize =
	//		pBlassArr[i]->dxrGeometryDesc.Triangles.IndexFormat == DXGI_FORMAT_R32_UINT ?
	//		sizeof(UINT32) : sizeof(UINT16);

	//	// Create Index SRV
	//	srvDesc.Buffer.NumElements =
	//		(iSingleIdexElementSize * pBlassArr[i]->dxrGeometryDesc.Triangles.IndexCount) / sizeof(UINT32); // 단순 인덱스 원소 개수가 아니라 UINT32로 얼마나 만들어지는지
	//	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	//	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	//	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	//	cpuHandle.Offset(1, iDescriptorSize);
	//	pDevice->CreateShaderResourceView(pBlassArr[i]->indexBuffer, &srvDesc, cpuHandle); // Index Srv
	//}
#pragma endregion Create Serial SRV of IB
#pragma region Create Serial SRV of VB
	for (UINT i = 0; i < iNumBlas; ++i)
	{
		// Create Vertex SRV
		srvDesc.Buffer.NumElements = pBlassArr[i]->dxrGeometryDesc.Triangles.VertexCount;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = pBlassArr[i]->dxrGeometryDesc.Triangles.VertexBuffer.StrideInBytes;
		cpuHandle.Offset(1, iDescriptorSize);
		pDevice->CreateShaderResourceView(pBlassArr[i]->vertexBuffer, &srvDesc, cpuHandle); // Vertex Srv
	}
#pragma endregion Create Serial SRV of VB
	// TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	DXR_Util::AllocateScratch_IfBigger(pDevice, topLevelPrebuildInfo.ScratchDataSizeInBytes);
	MyUtils::AllocateUAVBuffer(pDevice, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, ppOutUAV_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescArr = new D3D12_RAYTRACING_INSTANCE_DESC[1]{};
	for (UINT i = 0; i < iNumBlas; ++i)
	{
		instanceDescArr[i].Transform[0][0] = instanceDescArr[i].Transform[1][1] = instanceDescArr[i].Transform[2][2] = 1;
		instanceDescArr[i].InstanceMask = 1;
		instanceDescArr[i].AccelerationStructure = pBlassArr[i]->uav_BLAS->GetGPUVirtualAddress();
	}
	MyUtils::AllocateUploadBuffer(pDevice, instanceDescArr, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * 1, ppOutInstanceDescResource);
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	{
		topLevelInputs.InstanceDescs = (*ppOutInstanceDescResource)->GetGPUVirtualAddress();
		topLevelBuildDesc.Inputs = topLevelInputs;
		topLevelBuildDesc.DestAccelerationStructureData = (*ppOutUAV_TLAS)->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
	}
	pCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(*ppOutUAV_TLAS));


	Safe_Delete_Array(instanceDescArr);
	//----------------------


	//DXR_Util::Build_TLAS0(pDevice, pCommandList, &m_TLAS.uav_TLAS, &m_TLAS.pInstanceDesc, pBlasArr, iChildArrSize);
	Safe_Delete_Array(pBlasArr);

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
