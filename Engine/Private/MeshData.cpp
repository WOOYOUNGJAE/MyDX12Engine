#include "MeshData.h"
#include "DeviceResource.h"
#include "DXRResource.h"
#include "UploadBuffer.h"

CMeshData::CMeshData() :
	m_pDevice(CDeviceResource::Get_Instance()->Get_Device()),
	m_pCommandList(CDeviceResource::Get_Instance()->Get_CommandList())
{
}

CMeshData::CMeshData(CMeshData& rhs) : CBase(rhs),
m_pDevice(rhs.m_pDevice),
m_pCommandList(rhs.m_pCommandList),
m_vertexBufferCPU(rhs.m_vertexBufferCPU),
m_indexBufferCPU(rhs.m_indexBufferCPU),
m_vertexBufferGPU(rhs.m_vertexBufferGPU),
m_indexBufferGPU(rhs.m_indexBufferGPU),
m_vertexUploadBuffer(rhs.m_vertexUploadBuffer),
m_indexUploadBuffer(rhs.m_indexUploadBuffer),
m_vertexBufferView(rhs.m_vertexBufferView),
m_indexBufferView(rhs.m_indexBufferView),
m_iCbvSrvUavOffset(rhs.m_iCbvSrvUavOffset),
m_vecVertexData(rhs.m_vecVertexData),
m_vecIndexData(rhs.m_vecIndexData)
#if DXR_ON
,m_BLAS(rhs.m_BLAS)
#endif
{

}

HRESULT CMeshData::Initialize_Prototype()
{
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("MeshGeometry: Device Null");
		return E_FAIL;
	}

	return S_OK;
}

void CMeshData::Init_VBV_IBV()
{
	m_vertexBufferView.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = m_iVertexByteStride;
	m_vertexBufferView.SizeInBytes = m_iVertexBufferByteSize;

	m_indexBufferView.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
	m_indexBufferView.Format = m_IndexFormat;
	m_indexBufferView.SizeInBytes = m_iIndexBufferByteSize;
}
#if DXR_ON

void CMeshData::Build_BLAS(void* pIndexData, void* pVertexData, UINT64 iIndexDataSize, UINT64 iVertexDataSize)
{
	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
	m_BLAS.indexBuffer = m_indexBufferGPU;
	m_BLAS.vertexBuffer = m_vertexBufferGPU;

	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = CDXRResource::Get_Instance()->Get_refHeapHandle_CPU();
	UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();

	D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& refTriangles = m_BLAS.dxrGeometryDesc.Triangles;
	m_BLAS.dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_BLAS.dxrGeometryDesc.Triangles.Transform3x4 = 0;
	m_BLAS.dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

#pragma region Create SRV of IB, VB
	// Create Index SRV
	srvDesc.Buffer.NumElements = m_iNumIndices / sizeof(UINT32);
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	cpuHandle.Offset(1, iDescriptorSize);
	pDevice->CreateShaderResourceView(m_BLAS.indexBuffer, &srvDesc, cpuHandle); // Index Srv

	refTriangles.IndexFormat = m_IndexFormat;
	refTriangles.IndexCount = m_iNumIndices;
	refTriangles.IndexBuffer = m_BLAS.indexBuffer->GetGPUVirtualAddress();

	// Create Vertex SRV
	srvDesc.Buffer.NumElements = m_iNumVertices;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.StructureByteStride = UINT(Get_SingleVertexSize());
	cpuHandle.Offset(1, iDescriptorSize);
	pDevice->CreateShaderResourceView(m_BLAS.vertexBuffer, &srvDesc, cpuHandle); // Index Srv

	refTriangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	refTriangles.VertexCount = m_iNumVertices;
	refTriangles.VertexBuffer.StartAddress = m_BLAS.vertexBuffer->GetGPUVirtualAddress();
	refTriangles.VertexBuffer.StrideInBytes = UINT64(Get_SingleVertexSize());
#pragma endregion
	D3D12_RAYTRACING_GEOMETRY_AABBS_DESC& refAABBDesc = m_BLAS.dxrGeometryDesc.AABBs;



	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refBottomLevelInputs = bottomLevelBuildDesc.Inputs;
	refBottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refBottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	refBottomLevelInputs.NumDescs = 1;
	refBottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	refBottomLevelInputs.pGeometryDescs = &m_BLAS.dxrGeometryDesc;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&refBottomLevelInputs, &m_BLAS.prebuildInfo);
	if (m_BLAS.prebuildInfo.ResultDataMaxSizeInBytes == 0)
	{
		MSG_BOX("MeshData : Building BLAS Failed");
		return;
	}

	// Allocate Scratch Buffer if cur Blas ResultDataMaxSizeInBytes is Bigger;
	::DXR_Util::AllocateScratch_IfBigger(pDevice, m_BLAS.prebuildInfo.ResultDataMaxSizeInBytes);	

	// Allocate UAV Buffer, 실질적인 BLAS Allocate
	MyUtils::AllocateUAVBuffer(pDevice,
		m_BLAS.prebuildInfo.ResultDataMaxSizeInBytes,
		&m_BLAS.uav_BLAS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	UINT iNumPostBuilds = 0;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* postBuildArr = nullptr;
	bottomLevelBuildDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
	bottomLevelBuildDesc.DestAccelerationStructureData = m_BLAS.uav_BLAS->GetGPUVirtualAddress();
	// 진짜로 GPU에서 BLAS Build
	CDXRResource::BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, iNumPostBuilds, postBuildArr);
}
#endif
HRESULT CMeshData::Free()
{
	if (m_bIsPrototype == true)
	{
#if DXR_ON
		/*Safe_Release(m_AS_CPU.srv_Vertex);
		Safe_Release(m_AS_CPU.srv_Index);*/
		Safe_Release(m_BLAS.uav_BLAS);
#endif
		Safe_Release(m_vertexBufferCPU);
		Safe_Release(m_indexBufferCPU);
		Safe_Release(m_vertexBufferGPU);
		Safe_Release(m_indexBufferGPU);
		Safe_Release(m_vertexUploadBuffer);
		Safe_Release(m_indexUploadBuffer);
	}
	return S_OK;
}

D3D12_VERTEX_BUFFER_VIEW* CMeshData::Get_VertexBufferViewPtr()
{
	return &m_vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* CMeshData::Get_IndexBufferViewPtr()
{
	return &m_indexBufferView;
}

void CMeshData::Normalize_Vertices(CMeshData* pMeshData)
{
	// Normalize vertices
	Vector3 vmin(1000, 1000, 1000);
	Vector3 vmax(-1000, -1000, -1000);
	// AABB min max
	for (auto& v : pMeshData->Get_vecVertices()) 
	{
		vmin.x = XMMin(vmin.x, v.position.x);
		vmin.y = XMMin(vmin.y, v.position.y);
		vmin.z = XMMin(vmin.z, v.position.z);
		vmax.x = XMMax(vmax.x, v.position.x);
		vmax.y = XMMax(vmax.y, v.position.y);
		vmax.z = XMMax(vmax.z, v.position.z);
	}
	float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
	float dl = XMMax(XMMax(dx, dy), dz);
	float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
		cz = (vmax.z + vmin.z) * 0.5f;

	// normalize
	for (auto& v : pMeshData->Get_vecVertices())
	{
		v.position.x = (v.position.x - cx) / dl;
		v.position.y = (v.position.y - cy) / dl;
		v.position.z = (v.position.z - cz) / dl;
	}
}

void CMeshData::Normalize_Vertices(std::list<CMeshData*>& refMeshList)
{
	// Normalize vertices
	Vector3 vmin(1000, 1000, 1000);
	Vector3 vmax(-1000, -1000, -1000);
	// AABB min max
	for (auto& iterMesh : refMeshList)
	{
		for (auto& v : iterMesh->Get_vecVertices())
		{
			vmin.x = XMMin(vmin.x, v.position.x);
			vmin.y = XMMin(vmin.y, v.position.y);
			vmin.z = XMMin(vmin.z, v.position.z);
			vmax.x = XMMax(vmax.x, v.position.x);
			vmax.y = XMMax(vmax.y, v.position.y);
			vmax.z = XMMax(vmax.z, v.position.z);
		}
	}

	float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
	float dl = XMMax(XMMax(dx, dy), dz);
	float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
		cz = (vmax.z + vmin.z) * 0.5f;

	// normalize
	for (auto& iterMesh : refMeshList) {
		for (auto& v : iterMesh->Get_vecVertices())
		{
			v.position.x = (v.position.x - cx) / dl;
			v.position.y = (v.position.y - cy) / dl;
			v.position.z = (v.position.z - cz) / dl;
		}
	}
}

void CMeshData::Normalize_Vertices(std::vector<CMeshData*>& refVecMesh)
{
	// Normalize vertices
	Vector3 vmin(1000, 1000, 1000);
	Vector3 vmax(-1000, -1000, -1000);
	// AABB min max
	for (auto& iterMesh : refVecMesh)
	{
		for (auto& v : iterMesh->Get_vecVertices())
		{
			vmin.x = XMMin(vmin.x, v.position.x);
			vmin.y = XMMin(vmin.y, v.position.y);
			vmin.z = XMMin(vmin.z, v.position.z);
			vmax.x = XMMax(vmax.x, v.position.x);
			vmax.y = XMMax(vmax.y, v.position.y);
			vmax.z = XMMax(vmax.z, v.position.z);
		}
	}

	float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
	float dl = XMMax(XMMax(dx, dy), dz);
	float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
		cz = (vmax.z + vmin.z) * 0.5f;

	// normalize
	for (auto& iterMesh : refVecMesh) {
		for (auto& v : iterMesh->Get_vecVertices())
		{
			v.position.x = (v.position.x - cx) / dl;
			v.position.y = (v.position.y - cy) / dl;
			v.position.z = (v.position.z - cz) / dl;
		}
	}
}
