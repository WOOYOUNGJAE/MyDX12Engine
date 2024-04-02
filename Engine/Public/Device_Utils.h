#pragma once
#include "DXRResource.h"
#include "Engine_Defines.h"
#include "MeshData.h"

NAMESPACE_(Engine)
class CMeshData;

NAMESPACE_(MyUtils)
// iSize�� iAlignment ����� �ø�
inline UINT Align(UINT iSize, UINT iAlignment)
{
	return (iSize + (iAlignment - 1)) & ~(iAlignment - 1);
}

/// ���ؽ�, �ε��� ���� ��� ����
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

	// �⺻ ���� �ڿ� ����
	HRESULT hr = S_OK;
	hr = device->CreateCommittedResource(
		&temp_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&TempBuffer,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&defaultBuffer));

	if (FAILED(hr)) { return E_FAIL; }

	// GPU�� ���ۿ� ���翡 �غ� �ʿ��� �ӽ� ���ε� �� ����
	temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	hr = device->CreateCommittedResource(
		&temp_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&TempBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppUploadBuffer));

	if (FAILED(hr)) { return E_FAIL; }

	// �⺻ ���ۿ� ������ �ڷ� ����
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	// ���ۿ� �����ؼ� ������ �ڷ��� ũ�� (����Ʈ ����)�� ����
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	// �⺻ ���� �ڿ������� �ڷ� ���� ��û
	// UpdateSubresource : CPU�޸𸮸� �ӽ� ���ε� ���� �����ϰ� ID3D12CommandList::CopySubresourceRegion�� �ӽ� ���ε� ���� �ڷḦ buffer�� ����
	UpdateSubresources<1>(cmdList, defaultBuffer, *ppUploadBuffer, 0, 0, 1, &subResourceData);
	//
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	*ppOutResource = defaultBuffer; // 

	// �� �Լ� ȣ�� ���Ŀ��� uploadBuffer�� �����ؾ� ��
	// ���簡 �Ϸ�Ǿ����� Ȯ������ �Ŀ� �����ϸ� ��

	return hr;
}

// 256�� �ּ� ������� ���
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

inline D3D12_RAYTRACING_GEOMETRY_DESC Generate_AccelerationStructureDesc(CMeshData* pMeshData, ID3D12Resource* pSrvIndex, ID3D12Resource* pSrvVertex)
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES; // Fixed
	geometryDesc.Triangles.IndexBuffer = pSrvIndex->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = pMeshData->Num_Indices();
	geometryDesc.Triangles.IndexFormat = pMeshData->Get_IndexFormat();
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	geometryDesc.Triangles.VertexCount = pMeshData->Num_Vertices();
	geometryDesc.Triangles.VertexBuffer.StartAddress = pSrvVertex->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = pMeshData->Get_StrideInBytes();

	return geometryDesc;
}

// ������ ������ �Ҵ縸, ������ ������ ���α���
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

_NAMESPACE


#if DXR_ON

NAMESPACE_(DXR_Util)

// Build IB, VB SRV
inline DXR::ACCELERATION_STRUCTURE_CPU Build_AccelerationStructures_CPU(CMeshData* pMeshData, ID3D12Device* pDevice, CD3DX12_CPU_DESCRIPTOR_HANDLE& refCpuHandle, UINT iDescriptorSize)
{
	DXR::ACCELERATION_STRUCTURE_CPU as_CPU{};
	D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& refTriangles = as_CPU.dxrGeometryDesc.Triangles;
	as_CPU.dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	as_CPU.dxrGeometryDesc.Triangles.Transform3x4 = 0;
	as_CPU.dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // �ϴ� OPAQUE


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// Create Index SRV
	srvDesc.Buffer.NumElements = pMeshData->Num_Indices() / sizeof(UINT32);
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // �ε����� �ܼ� ���� �����̹Ƿ� rawŸ������
	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, �� ���� �����ͷ� ������ ��
	refCpuHandle.Offset(1, iDescriptorSize);
	pDevice->CreateShaderResourceView(as_CPU.srv_Index, &srvDesc, refCpuHandle); // Index Srv

	refTriangles.IndexFormat = pMeshData->Get_IndexFormat();
	refTriangles.IndexCount = pMeshData->Num_Indices();
	refTriangles.IndexBuffer = as_CPU.srv_Index->GetGPUVirtualAddress();

	// Create Vertex SRV
	srvDesc.Buffer.NumElements = pMeshData->Num_Vertices();
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.StructureByteStride = UINT(pMeshData->Get_SingleVertexSize());
	refCpuHandle.Offset(1, iDescriptorSize);
	pDevice->CreateShaderResourceView(as_CPU.srv_Vertex, &srvDesc, refCpuHandle); // Index Srv

	refTriangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	refTriangles.VertexCount = pMeshData->Num_Vertices();
	refTriangles.VertexBuffer.StartAddress = as_CPU.srv_Vertex->GetGPUVirtualAddress();
	refTriangles.VertexBuffer.StrideInBytes = UINT64(pMeshData->Get_SingleVertexSize());

	return as_CPU;
}

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

inline void Build_TLAS(ID3D12Device5* pDevice, ID3D12Resource** ppOutUAV_TLAS, ID3D12Resource** ppOutInstanceDescResource, ID3D12Resource** pUAV_BLASArr, UINT iNumBlas)
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

	// Allocate UAV Buffer, �������� TLAS
	MyUtils::AllocateUAVBuffer(pDevice, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, ppOutUAV_TLAS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	// Create instance descs for the bottom-level acceleration structures.
	D3D12_RAYTRACING_INSTANCE_DESC* instanceDescArr = new D3D12_RAYTRACING_INSTANCE_DESC[iNumBlas]{};

	for (UINT i = 0; i < iNumBlas; ++i)
	{
		instanceDescArr[i].Transform[0][0] = instanceDescArr[i].Transform[1][1] = instanceDescArr[i].Transform[2][2] = 1;
		instanceDescArr[i].InstanceMask = 1;
		instanceDescArr[i].AccelerationStructure = pUAV_BLASArr[i]->GetGPUVirtualAddress();
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


_NAMESPACE//DXR_Util

#endif DXR_ON

_NAMESPACE