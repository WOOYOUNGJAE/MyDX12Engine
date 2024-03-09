#include "MeshData.h"
#include "Graphic_Device.h"
#include "UploadBuffer.h"

CMeshData::CMeshData() :
	m_pDevice(CGraphic_Device::Get_Instance()->Get_Device()),
	m_pCommandList(CGraphic_Device::Get_Instance()->Get_CommandList())
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

HRESULT CMeshData::Free()
{
	if (m_bIsPrototype == true)
	{
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
