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
m_vertexPosArr(rhs.m_vertexPosArr),
m_vertexBufferCPU(rhs.m_vertexBufferCPU),
m_indexBufferCPU(rhs.m_indexBufferCPU),
m_vertexBufferGPU(rhs.m_vertexBufferGPU),
m_indexBufferGPU(rhs.m_indexBufferGPU),
m_vertexUploadBuffer(rhs.m_vertexUploadBuffer),
m_indexUploadBuffer(rhs.m_indexUploadBuffer),
m_vertexBufferView(rhs.m_vertexBufferView),
m_indexBufferView(rhs.m_indexBufferView),
m_iCbvSrvUavOffset(rhs.m_iCbvSrvUavOffset)
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
	m_indexBufferView.Format = IndexFormat;
	m_indexBufferView.SizeInBytes = m_iIndexBufferByteSize;
}

HRESULT CMeshData::Free()
{
	if (m_iClonedNum == 0)
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
	/*D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = m_iVertexByteStride;
	vbv.SizeInBytes = m_iVertexBufferByteSize;*/

	return &m_vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* CMeshData::Get_IndexBufferViewPtr()
{
	/*D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = IndexFormat;
	ibv.SizeInBytes = m_iIndexBufferByteSize;*/

	return &m_indexBufferView;
}
