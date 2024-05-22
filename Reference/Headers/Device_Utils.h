#pragma once
#include "DXRResource.h"
#include "Engine_Defines.h"
#include "GameObject.h"
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

void Build_BLAS(ID3D12Device5* pDevice, ID3D12GraphicsCommandList4* pCommandList, DXR::BLAS* pBLAS,
	ID3D12Resource* pIndexBuffer, ID3D12Resource* pVertexBuffer, DXGI_FORMAT IndexFormat, UINT iTriangleIndexCount, UINT iTriangleVertexCount, UINT64 iVertexStrideInBytes);

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
void Create_IB_VB_SRV_Serialized(ID3D12Device5* pDevice, UINT iNumAllIndices, UINT iNumAllVertices, ID3D12Resource* pCombinedIndicesResource, ID3D12Resource*
                                 pCombinedVerticesResource, UINT iStructureByteStride, UINT64* pOutIBStartOffsetInDescriptors);

void Build_TLAS0(ID3D12Device5* pDevice, ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource** ppOutUAV_TLAS, ID3D12Resource** ppOutInstanceDescResource, CGameObject
	** pGameObjArr, UINT
	* iNumberingArr, UINT iNumBlas);

void Update_ShaderRecord(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pSrcResource, ID3D12Resource* pDstShaderTable, UINT64 ShaderIDSize, UINT
                         iNumRecords, UINT iSingleArgumentSize);


_NAMESPACE//DXR_Util

#endif DXR_ON

_NAMESPACE