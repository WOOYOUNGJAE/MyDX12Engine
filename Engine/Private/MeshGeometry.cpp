#include "MeshGeometry.h"
#include "Graphic_Device.h"
#include "UploadBuffer.h"

CMeshGeometry::CMeshGeometry() :
	m_pDevice(CGraphic_Device::Get_Instance()->Get_Device()),
	m_pCommandList(CGraphic_Device::Get_Instance()->Get_CommandList())
{
}

CMeshGeometry::CMeshGeometry(CMeshGeometry& rhs) : CComponent(rhs),
m_pDevice(rhs.m_pDevice),
m_pCommandList(rhs.m_pCommandList),
m_vertexPosArr(rhs.m_vertexPosArr),
//m_pResource(rhs.m_pResource.Get()),
m_vertexBufferCPU(rhs.m_vertexBufferCPU.Get()),
m_indexBufferCPU(rhs.m_indexBufferCPU.Get()),
m_vertexBufferGPU(rhs.m_vertexBufferGPU.Get()),
m_indexBufferGPU(rhs.m_indexBufferGPU.Get()),
m_vertexUploadBuffer(rhs.m_vertexUploadBuffer.Get()),
m_indexUploadBuffer(rhs.m_indexUploadBuffer.Get())
{
}

HRESULT CMeshGeometry::Initialize_Prototype()
{
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("MeshGeometry: Device Null");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMeshGeometry::Free()
{
	/*Safe_Delete_Array(m_vertexData);
	Safe_Delete_Array(m_vertexPosArr);*/
	

	if (FAILED(CComponent::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}

D3D12_VERTEX_BUFFER_VIEW CMeshGeometry::VertexBufferView()
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = m_iVertexByteStride;
	vbv.SizeInBytes = m_iVertexBufferByteSize;

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW CMeshGeometry::IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = IndexFormat;
	ibv.SizeInBytes = m_iIndexBufferByteSize;

	return ibv;
}
