#pragma once
#include "DXRResource.h"
#include "Engine_Defines.h"
#include "MeshData.h"
#include "SceneNode_AABB.h"

NAMESPACE_(Engine)
	class CMeshData;

NAMESPACE_(MyUtils)
// iSize를 iAlignment 배수로 올림
inline UINT Align(UINT iSize, UINT iAlignment)
{
	return (iSize + (iAlignment - 1)) & ~(iAlignment - 1);
}

/// 버텍스, 인덱스 버퍼 모두 가능
inline HRESULT Create_Buffer_Default(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	ID3D12Resource** ppUploadBuffer,
	ID3D12Resource** ppOutResource)
{
	ID3D12Resource* defaultBuffer;

	CD3DX12_HEAP_PROPERTIES temp_heap_properties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC TempBuffer =
		CD3DX12_RESOURCE_DESC::Buffer(byteSize);

	// 기본 버퍼 자원 생성
	HRESULT hr = S_OK;
	hr = device->CreateCommittedResource(
		&temp_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&TempBuffer,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&defaultBuffer));

	if (FAILED(hr)) { return E_FAIL; }

	// GPU의 버퍼에 복사에 준비에 필요한 임시 업로드 힙 생성
	temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	hr = device->CreateCommittedResource(
		&temp_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&TempBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppUploadBuffer));

	if (FAILED(hr)) { return E_FAIL; }

	// 기본 버퍼에 복사할 자료 서술
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	// 버퍼에 한정해서 복사할 자료의 크기 (바이트 개수)를 뜻함
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	// 기본 버퍼 자원으로의 자료 복사 요청
	// UpdateSubresource : CPU메모리를 임시 업로드 힙에 복사하고 ID3D12CommandList::CopySubresourceRegion로 임시 업로드 힙의 자료를 buffer에 복사
	UpdateSubresources<1>(cmdList, defaultBuffer, *ppUploadBuffer, 0, 0, 1, &subResourceData);
	//
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	*ppOutResource = defaultBuffer; // 

	// 이 함수 호출 이후에도 uploadBuffer를 유지해야 함
	// 복사가 완료되었음이 확실해진 후에 해제하면 됨

	return hr;
}

// 256의 최소 공배수로 계산
inline _uint Align256(_uint iByteSize)
{
	return MyUtils::Align(iByteSize, 256);
}

inline ComPtr<ID3DBlob> CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;

	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);


	if (FAILED(hr))
	{
		MSG_BOX("Failed to Compile Shader");

		if (errors != nullptr)
			OutputDebugStringA((char*)errors->GetBufferPointer());
	}


	return byteCode;
}

// 데이터 없으면 할당만, 있으면 데이터 맵핑까지
inline HRESULT AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr)
{
	HRESULT hr = S_OK;

	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	hr = pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource));
	if (FAILED(hr)) { return hr; }

	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}

	if (pData) 
	{
		void* pMappedData;
		(*ppResource)->Map(0, nullptr, &pMappedData);
		memcpy(pMappedData, pData, datasize);
		(*ppResource)->Unmap(0, nullptr);		
	}

	return hr;
}
inline HRESULT AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
{
	HRESULT hr = S_OK;
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	hr = pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}

	return hr;
}

inline UINT64 Get_HeapStartOffset(D3D12_CPU_DESCRIPTOR_HANDLE startHandleCPU, D3D12_CPU_DESCRIPTOR_HANDLE curHandleCPU, UINT iDescriptorSize)
{
	return (curHandleCPU.ptr - startHandleCPU.ptr) / iDescriptorSize;
}

_NAMESPACE


#if DXR_ON

NAMESPACE_(DXR_Util)
	inline void AllocateScratch_IfBigger(ID3D12Device5* pDevice, UINT64 newWidth)
{
	ID3D12Resource** ppScratchBuffer = CDXRResource::Get_Instance()->Get_ScratchBufferPtr();
	UINT64 iPrevWidth = 0;
	if (*ppScratchBuffer)
	{
		iPrevWidth = (*ppScratchBuffer)->GetDesc().Width;;
	}
	UINT64 iWidth = max(iPrevWidth, newWidth);

	if (iWidth > iPrevWidth)
	{
		Safe_Release(*ppScratchBuffer);
		if (*ppScratchBuffer) { MSG_BOX("MeshData : Releasing Scratch Failed"); }

		MyUtils::AllocateUAVBuffer(pDevice, iWidth, ppScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	}
}

inline void Build_TLAS(ID3D12Device5* pDevice, ID3D12Resource** ppOutUAV_TLAS, ID3D12Resource** ppOutInstanceDescResource, ID3D12Resource***
                       ppUAV_BLASArr, UINT iNumBlas)
{
	ID3D12Resource*& pScratch = CDXRResource::Get_Instance()->Get_ScratchBufferRef();

	// Get required sizes for an acceleration structure.
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refTopLevelInputs = topLevelBuildDesc.Inputs;
	refTopLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	refTopLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refTopLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	refTopLevelInputs.NumDescs = iNumBlas;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&refTopLevelInputs, &topLevelPrebuildInfo);
	if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0) { MSG_BOX("Failed to Bulid TLAS"); return; }

	::Engine::DXR_Util::AllocateScratch_IfBigger(pDevice, topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	// Allocate UAV Buffer, 실질적인 TLAS
	MyUtils::AllocateUAVBuffer(pDevice, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, ppOutUAV_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	// Create instance descs for the bottom-level acceleration structures.
	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescArr = new D3D12_RAYTRACING_INSTANCE_DESC[iNumBlas]{};

	for (UINT i = 0; i < iNumBlas; ++i)
	{
		instanceDescArr[i].Transform[0][0] = instanceDescArr[i].Transform[1][1] = instanceDescArr[i].Transform[2][2] = 1;
		instanceDescArr[i].InstanceMask = 1;
		instanceDescArr[i].AccelerationStructure = (*ppUAV_BLASArr[i])->GetGPUVirtualAddress();
	}

	MyUtils::AllocateUploadBuffer(pDevice, instanceDescArr, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * iNumBlas, ppOutInstanceDescResource);

	// Top-level AS desc
	{
		topLevelBuildDesc.DestAccelerationStructureData = (*ppOutUAV_TLAS)->GetGPUVirtualAddress();
		refTopLevelInputs.InstanceDescs = (*ppOutInstanceDescResource)->GetGPUVirtualAddress();
		topLevelBuildDesc.ScratchAccelerationStructureData = pScratch->GetGPUVirtualAddress();
	}

	// Build acceleration structure.
	CDXRResource::BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	Safe_Delete_Array(instanceDescArr);
}
inline void Create_IB_VB_SRV_Serialized(ID3D12Device5* pDevice, DXR::BLAS** pBlassArr, UINT iNumBlas)
{
// 모든 BLAS에 대한 Index SRV를 연속적으로 만든 후 Vertex SRV 만들기
	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = CDXRResource::Get_Instance()->Get_refHeapHandle_CPU();
	UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#pragma region Create Serial SRV of IB
	for (UINT i = 0; i < iNumBlas; ++i)
	{
		UINT iSingleIdexElementSize =
			pBlassArr[i]->dxrGeometryDesc.Triangles.IndexFormat == DXGI_FORMAT_R32_UINT ?
			sizeof(UINT32) : sizeof(UINT16);

		// Create Index SRV
		srvDesc.Buffer.NumElements =
			(iSingleIdexElementSize * pBlassArr[i]->dxrGeometryDesc.Triangles.IndexCount) / sizeof(UINT32); // 단순 인덱스 원소 개수가 아니라 UINT32로 얼마나 만들어지는지
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
		srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
		cpuHandle.Offset(1, iDescriptorSize);
		pDevice->CreateShaderResourceView(pBlassArr[i]->indexBuffer, &srvDesc, cpuHandle); // Index Srv
	}
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
}

inline void Build_TLAS0(ID3D12Device5* pDevice, ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource** ppOutUAV_TLAS, ID3D12Resource** ppOutInstanceDescResource, DXR::BLAS** pBlassArr, UINT iNumBlas)
{
	Create_IB_VB_SRV_Serialized(pDevice, pBlassArr, iNumBlas);

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
}

_NAMESPACE//DXR_Util

#endif DXR_ON

_NAMESPACE