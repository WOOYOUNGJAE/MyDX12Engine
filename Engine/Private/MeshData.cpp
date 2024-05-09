#include "MeshData.h"
#include "DeviceResource.h"
#include "DXRResource.h"
#include "UploadBuffer.h"

CMeshData::CMeshData() :
	m_pDevice(CDeviceResource::Get_Instance()->Get_Device()),
#if DXR_ON
	m_pCommandList(CDXRResource::Get_Instance()->Get_CommandList4())
#else
	m_pCommandList(CDeviceResource::Get_Instance()->Get_CommandList())
#endif
{
#if DXR_ON
	ZeroMemory(&m_BLAS, sizeof(DXR::BLAS));
#endif
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

void CMeshData::Build_BLAS(UINT64 iIndexDataSize, UINT64 iVertexDataSize)
{
	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
	m_BLAS.indexBuffer = m_indexBufferGPU;
	m_BLAS.vertexBuffer = m_vertexBufferGPU;

#pragma region Create SRV of IB, VB
	//CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = CDXRResource::Get_Instance()->Get_refHeapHandle_CPU();
	//UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();

	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//// Create Index SRV
	//srvDesc.Buffer.NumElements = iIndexDataSize / sizeof(UINT32);
	//srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	//srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	//cpuHandle.Offset(1, iDescriptorSize);
	//pDevice->CreateShaderResourceView(m_BLAS.indexBuffer, &srvDesc, cpuHandle); // Index Srv

	//// Create Vertex SRV
	//srvDesc.Buffer.NumElements = m_iNumVertices;
	//srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//srvDesc.Buffer.StructureByteStride = UINT(Get_SingleVertexSize());
	//cpuHandle.Offset(1, iDescriptorSize);
	//pDevice->CreateShaderResourceView(m_BLAS.vertexBuffer, &srvDesc, cpuHandle); // Index Srv
#pragma endregion
	// DXR Geometry Desc
	m_BLAS.dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_BLAS.dxrGeometryDesc.Triangles.Transform3x4 = 0;
	m_BLAS.dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE

	m_BLAS.dxrGeometryDesc.Triangles.IndexFormat = m_IndexFormat;
	m_BLAS.dxrGeometryDesc.Triangles.IndexCount = m_iNumIndices;
	m_BLAS.dxrGeometryDesc.Triangles.IndexBuffer = m_BLAS.indexBuffer->GetGPUVirtualAddress();

	m_BLAS.dxrGeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	m_BLAS.dxrGeometryDesc.Triangles.VertexCount = m_iNumVertices;
	m_BLAS.dxrGeometryDesc.Triangles.VertexBuffer.StartAddress = m_BLAS.vertexBuffer->GetGPUVirtualAddress();
	m_BLAS.dxrGeometryDesc.Triangles.VertexBuffer.StrideInBytes = UINT64(Get_SingleVertexSize());

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refBottomLevelInputs = m_BLAS.accelerationStructureDesc.Inputs;
	refBottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refBottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	refBottomLevelInputs.NumDescs = 1;
	refBottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	refBottomLevelInputs.pGeometryDescs = &m_BLAS.dxrGeometryDesc;

	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&m_BLAS.accelerationStructureDesc.Inputs, &m_BLAS.prebuildInfo);
	if (m_BLAS.prebuildInfo.ResultDataMaxSizeInBytes == 0)
	{
		MSG_BOX("MeshData : Building BLAS Failed");
		return;
	}

	// Allocate Scratch Buffer if cur Blas ResultDataMaxSizeInBytes is Bigger;
	::DXR_Util::AllocateScratch_IfBigger(pDevice, m_BLAS.prebuildInfo.ResultDataMaxSizeInBytes);	

	// Allocate UAV Buffer, 실질적인 BLAS Allocate
	HRESULT hr = MyUtils::AllocateUAVBuffer(pDevice,
		m_BLAS.prebuildInfo.ResultDataMaxSizeInBytes,
		//&m_BLAS.uav_BLAS,
		&m_BLAS.uav_BLAS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	if (FAILED(hr)) { MSG_BOX("Allocate Failed"); }

	m_BLAS.accelerationStructureDesc.ScratchAccelerationStructureData = (*CDXRResource::Get_Instance()->Get_ScratchBufferPtr())->GetGPUVirtualAddress();
	m_BLAS.accelerationStructureDesc.DestAccelerationStructureData = m_BLAS.uav_BLAS->GetGPUVirtualAddress();

	m_pCommandList->BuildRaytracingAccelerationStructure(&m_BLAS.accelerationStructureDesc, 0, nullptr);
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_BLAS.uav_BLAS));
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
