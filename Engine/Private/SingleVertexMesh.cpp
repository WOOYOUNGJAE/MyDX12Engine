#include "SingleVertexMesh.h"
#include "Device_Utils.h"

CSingleVertexMesh::CSingleVertexMesh()
{
	m_iVertexByteStride = sizeof(VertexPositionSize);
	m_IndexFormat = IMeshDataType::IndexFormat;
	m_iNumVertices = 1;
	m_iNumIndices = 1;
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT16);
}

CSingleVertexMesh::CSingleVertexMesh(CSingleVertexMesh& rhs) : CMeshData(rhs), IMeshDataType(rhs)
{
	Safe_AddRef(m_vertexBufferCPU);
	Safe_AddRef(m_indexBufferCPU);
	Safe_AddRef(m_vertexBufferGPU);
	Safe_AddRef(m_indexBufferGPU);
	Safe_AddRef(m_vertexUploadBuffer);
	Safe_AddRef(m_indexUploadBuffer);

	m_iNumVertices = rhs.m_iNumVertices;
	m_iNumIndices = rhs.m_iNumIndices;
	m_iVertexByteStride = rhs.m_iVertexByteStride;
	m_iVertexBufferByteSize = rhs.m_iVertexBufferByteSize;
	m_IndexFormat = rhs.m_IndexFormat;
	m_iIndexBufferByteSize = rhs.m_iIndexBufferByteSize;
}

CSingleVertexMesh* CSingleVertexMesh::Create()
{
	CSingleVertexMesh* pInstance = new CSingleVertexMesh();

	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init BillboardMesh Prototype");
	}

	return pInstance;
}

CMeshData* CSingleVertexMesh::Clone(void* pArg)
{
	CSingleVertexMesh* pInstance = new CSingleVertexMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("BillboardMesh : Failed to Clone");
		return nullptr;
	}
	return pInstance;
}

HRESULT CSingleVertexMesh::Initialize_Prototype()
{
	HRESULT hr = CMeshData::Initialize_Prototype();


	VertexPositionSize v = VertexPositionSize(Vector3::Zero, Vector2(1.f, 1.f));
	IMeshDataType::vecVertexData.push_back(v);
	vecVertexData.shrink_to_fit();

	vecIndexData.push_back(0);
	vecIndexData.shrink_to_fit();

	const UINT iVertexBufferSize = sizeof(VertexPositionSize) * m_iNumVertices;
	const UINT iIndexBufferSize = sizeof(UINT16) * m_iNumIndices;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), vecIndexData.data(), iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		vecIndexData.data(), iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
	return hr;
}

HRESULT CSingleVertexMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CSingleVertexMesh::Free()
{
	if (m_bIsPrototype == false)
	{
		Safe_Release(m_vertexBufferCPU);
		Safe_Release(m_indexBufferCPU);
		Safe_Release(m_vertexBufferGPU);
		Safe_Release(m_indexBufferGPU);
		Safe_Release(m_vertexUploadBuffer);
		Safe_Release(m_indexUploadBuffer);
	}

	return CMeshData::Free();
}

DXGI_FORMAT CSingleVertexMesh::Get_IndexFormat()
{
	return IMeshDataType::IndexFormat;
}
