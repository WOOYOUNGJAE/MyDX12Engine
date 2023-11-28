#include "MeshGeometry.h"
#include "Graphic_Device.h"
#include "UploadBuffer.h"

CMeshGeometry::CMeshGeometry() :
	m_pDevice(CGraphic_Device::Get_Instance()->Get_Device()),
	m_pCommandList(CGraphic_Device::Get_Instance()->Get_CommandList())
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
