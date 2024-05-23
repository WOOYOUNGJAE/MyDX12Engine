#include "Device_Utils.h"

void DXR_Util::Build_BLAS(ID3D12Device5* pDevice, ID3D12GraphicsCommandList4* pCommandList, DXR::BLAS* pBLAS,
	ID3D12Resource* pIndexBuffer, ID3D12Resource* pVertexBuffer, DXGI_FORMAT IndexFormat, UINT iTriangleIndexCount, UINT iTriangleVertexCount, UINT64 iVertexStrideInBytes)
{
	pBLAS->indexBuffer = pIndexBuffer;
	pBLAS->vertexBuffer = pVertexBuffer;

	// DXR Geometry Desc
	D3D12_RAYTRACING_GEOMETRY_DESC dxrGeometryDesc{};
	pBLAS->dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	pBLAS->dxrGeometryDesc.Triangles.Transform3x4 = 0;
	pBLAS->dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE

	pBLAS->dxrGeometryDesc.Triangles.IndexFormat = IndexFormat;
	pBLAS->dxrGeometryDesc.Triangles.IndexCount = iTriangleIndexCount;
	pBLAS->dxrGeometryDesc.Triangles.IndexBuffer = pBLAS->indexBuffer->GetGPUVirtualAddress();

	pBLAS->dxrGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	pBLAS->dxrGeometryDesc.Triangles.VertexCount = iTriangleVertexCount;
	pBLAS->dxrGeometryDesc.Triangles.VertexBuffer.StartAddress = pBLAS->vertexBuffer->GetGPUVirtualAddress();
	pBLAS->dxrGeometryDesc.Triangles.VertexBuffer.StrideInBytes = iVertexStrideInBytes;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refBottomLevelInputs = pBLAS->accelerationStructureDesc.Inputs;
	refBottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refBottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	refBottomLevelInputs.NumDescs = 1;
	refBottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	refBottomLevelInputs.pGeometryDescs = &(pBLAS->dxrGeometryDesc);

	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&pBLAS->accelerationStructureDesc.Inputs, &pBLAS->prebuildInfo);
	if (pBLAS->prebuildInfo.ResultDataMaxSizeInBytes == 0)
	{
		MSG_BOX("MeshData : Building BLAS Failed");
		return;
	}

	// Allocate Scratch Buffer if cur Blas ResultDataMaxSizeInBytes is Bigger;
	::DXR_Util::AllocateScratch_IfBigger(pDevice, pBLAS->prebuildInfo.ResultDataMaxSizeInBytes);

	// Allocate UAV Buffer, 실질적인 BLAS Allocate
	HRESULT hr = MyUtils::AllocateUAVBuffer(pDevice,
		pBLAS->prebuildInfo.ResultDataMaxSizeInBytes,
		&pBLAS->uav_BLAS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	if (FAILED(hr)) { MSG_BOX("Allocate Failed"); }

	pBLAS->accelerationStructureDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
	pBLAS->accelerationStructureDesc.DestAccelerationStructureData = pBLAS->uav_BLAS->GetGPUVirtualAddress();

	pCommandList->BuildRaytracingAccelerationStructure(&pBLAS->accelerationStructureDesc, 0, nullptr);
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(pBLAS->uav_BLAS));
}

void DXR_Util::Create_IB_VB_SRV_Serialized(ID3D12Device5* pDevice,
                                           UINT iNumAllIndices, UINT iNumAllVertices, ID3D12Resource* pCombinedIndicesResource, ID3D12Resource* pCombinedVerticesResource, UINT
                                           iStructureByteStride, UINT64* pOutIBStartOffsetInDescriptors)
{
	// 모든 BLAS에 대한 Index SRV를 연속적으로 만든 후 Vertex SRV 만들기
	CDXRResource* pDXRResource = CDXRResource::Get_Instance();
	*pOutIBStartOffsetInDescriptors = pDXRResource->Get_CurOffsetInDescriptors();
	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = pDXRResource->Get_refHeapHandle_CPU();
	UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#pragma region Create Serial SRV of IB
	UINT iSingleIdexElementSize = sizeof(UINT16);

	// Create Index SRV
	srvDesc.Buffer.NumElements =
		(iSingleIdexElementSize * iNumAllIndices) / sizeof(UINT32); // 단순 인덱스 원소 개수가 아니라 UINT32로 얼마나 만들어지는지, D3D12_BUFFER_SRV_FLAG_RAW이라서
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	//cpuHandle.Offset(1, iDescriptorSize);
	pDevice->CreateShaderResourceView(pCombinedIndicesResource, &srvDesc, cpuHandle); // Index Srv
	pDXRResource->Apply_DescriptorHandleOffset();
#pragma endregion Create Serial SRV of IB
#pragma region Create Serial SRV of VB
	// Create Vertex SRV
	srvDesc.Buffer.NumElements = iNumAllVertices;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.StructureByteStride = iStructureByteStride;
	auto a = pCombinedVerticesResource->GetDesc().Width;
	pDevice->CreateShaderResourceView(pCombinedVerticesResource, &srvDesc, cpuHandle); // Vertex Srv
	pDXRResource->Apply_DescriptorHandleOffset();
#pragma endregion Create Serial SRV of VB	
}


void DXR_Util::Build_TLAS0(ID3D12Device5* pDevice, ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource** ppOutUAV_TLAS, ID3D12Resource** ppOutInstanceDescResource, CGameObject
                           ** pGameObjArr, UINT
                           * iNumberingArr, UINT iNumBlas)

{
	// TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = iNumBlas;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	DXR_Util::AllocateScratch_IfBigger(pDevice, topLevelPrebuildInfo.ScratchDataSizeInBytes);
	MyUtils::AllocateUAVBuffer(pDevice, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, ppOutUAV_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescArr = new D3D12_RAYTRACING_INSTANCE_DESC[iNumBlas]{};
	for (UINT i = 0; i < iNumBlas; ++i)
	{
		instanceDescArr[i].InstanceID = iNumberingArr[i] - 1;
		instanceDescArr[i].InstanceMask = 1;
		instanceDescArr[i].AccelerationStructure = pGameObjArr[i]->Get_BLAS_Resource()->GetGPUVirtualAddress();

		Matrix worldMat = pGameObjArr[i]->Get_WorldMatrix();
		Vector3 pos = pGameObjArr[i]->Get_Pos();
		memcpy(instanceDescArr[i].Transform[0], &worldMat.m[0], sizeof(FLOAT) * 3);
		memcpy(instanceDescArr[i].Transform[1], &worldMat.m[1], sizeof(FLOAT) * 3);
		memcpy(instanceDescArr[i].Transform[2], &worldMat.m[2], sizeof(FLOAT) * 3);
		instanceDescArr[i].Transform[0][3] = pos.x;
		instanceDescArr[i].Transform[1][3] = pos.y;
		instanceDescArr[i].Transform[2][3] = pos.z;

		//instanceDescArr[i].Transform[0][0] = instanceDescArr[i].Transform[1][1] = instanceDescArr[i].Transform[2][2] = 1;
	}
	MyUtils::AllocateUploadBuffer(pDevice, instanceDescArr, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * iNumBlas, ppOutInstanceDescResource);
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
}

void DXR_Util::Update_ShaderRecord(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pSrcResource,
                                   ID3D12Resource* pDstShaderTable, UINT64 ShaderIDSize, UINT iNumRecords, UINT iSingleArgumentSize, UINT
                                   iOffsetBytesInArguments)
{
	for (UINT iRecordIndex = 0; iRecordIndex < iNumRecords; ++iRecordIndex)
	{
		pCommandList->CopyBufferRegion(pDstShaderTable,
// |ID|...OffsetBytesInArguments|SingleArgumentSize(0)|SingleArgumentSize(1)|SingleArgumentSize(2)|..
			ShaderIDSize + iOffsetBytesInArguments + UINT64(iSingleArgumentSize) * iRecordIndex,
			pSrcResource,
			UINT64(MyUtils::Align256(iSingleArgumentSize) * iRecordIndex),
			iSingleArgumentSize);
	}
}

void DXR_Util::Update_ShaderRecord(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pSrcResource,
	ID3D12Resource* pDstShaderTable, UINT64 ShaderIDSize, UINT iNumRecords, UINT iSingleArgumentSize)
{
	for (UINT iRecordIndex = 0; iRecordIndex < iNumRecords; ++iRecordIndex)
	{
		pCommandList->CopyBufferRegion(pDstShaderTable,
			UINT64(iSingleArgumentSize) * iRecordIndex + ShaderIDSize,
			pSrcResource,
			UINT64(MyUtils::Align256(iSingleArgumentSize) * iRecordIndex),
			iSingleArgumentSize);
	}
}