#pragma once
#include "DXRResource.h"
#include "Engine_Defines.h"
#include "MeshData.h"

NAMESPACE_(Engine)
class CMeshData;


class CDevice_Utils
{
private:
	CDevice_Utils() = default;
	~CDevice_Utils() = default;

public:
	/// 버텍스, 인덱스 버퍼 모두 가능
	static HRESULT Create_Buffer_Default(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ID3D12Resource** ppUploadBuffer,
		ID3D12Resource** ppOutResource);

	// 256의 최소 공배수로 계산
	static _uint ConstantBufferByteSize(_uint iByteSize)
	{
		return (iByteSize + 255) & ~255;
	}

	static ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

	static D3D12_RAYTRACING_GEOMETRY_DESC Generate_AccelerationStructureDesc(CMeshData* pMeshData, ID3D12Resource* pSrvIndex, ID3D12Resource* pSrvVertex);
};

inline HRESULT AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr)
{
	HRESULT hr = S_OK;
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	hr = pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
	void* pMappedData;
	(*ppResource)->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	(*ppResource)->Unmap(0, nullptr);

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

#if DXR_ON

NAMESPACE_(DXR_Util)

// Build IB, VB SRV
inline DXR::ACCELERATION_STRUCTURE_CPU Build_AccelerationStructures_CPU(CMeshData* pMeshData, ID3D12Device* pDevice, CD3DX12_CPU_DESCRIPTOR_HANDLE& refCpuHandle, UINT iDescriptorSize)
{
	DXR::ACCELERATION_STRUCTURE_CPU as_CPU{};
	D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& refTriangles = as_CPU.dxrGeometryDesc.Triangles;
	as_CPU.dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	as_CPU.dxrGeometryDesc.Triangles.Transform3x4 = 0;
	as_CPU.dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// Create Index SRV
	srvDesc.Buffer.NumElements = pMeshData->Num_Indices() / sizeof(UINT32);
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
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

		AllocateUAVBuffer(pDevice, iWidth, ppScratchBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	}
}

void Build_TLAS(ID3D12Device5* pDevice, ID3D12Resource** ppOutUAV_TLAS, ID3D12Resource** pUAV_BLASArr, UINT iNumBlas);


_NAMESPACE

#endif DXR_ON

_NAMESPACE