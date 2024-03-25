#include "AssetMesh.h"

#include "Device_Utils.h"

CAssetMesh::CAssetMesh(CAssetMesh& rhs) : CMeshData(rhs),
m_strPath(rhs.m_strPath)
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

CAssetMesh* CAssetMesh::Create()
{
	CAssetMesh* pInstance = new CAssetMesh();
	pInstance->m_bIsPrototype = true;

	pInstance->Initialize_Prototype();

	return pInstance;
}

CMeshData* CAssetMesh::Clone(void* pArg)
{
	CAssetMesh* pInstance = new CAssetMesh(*this);

	pInstance->Initialize(pArg);

	return pInstance;
}

HRESULT CAssetMesh::Initialize_Prototype()
{
	return CMeshData::Initialize_Prototype();
}

HRESULT CAssetMesh::ReInit_Prototype()
{
	HRESULT hr = S_OK;

	//Normalize_Vertices(this);

	m_iNumVertices = m_vecVertexData.size();
	m_iNumIndices = m_vecIndexData.size();
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT32);

	const UINT iVertexBufferSize = sizeof(VertexPositionNormalTexture) * m_iNumVertices;
	const UINT iIndexBufferSize = sizeof(UINT32) * m_iNumIndices;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("AssetMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("AssetMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), m_vecIndexData.data(), iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecIndexData.data(), iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	CMeshData::Build_BLAS(m_vecIndexData.data(), m_vecVertexData.data(),
		sizeof(UINT32) * Num_Indices(), sizeof(VertexPositionNormalTexture) * m_iNumVertices);
#endif
	return hr;
}

HRESULT CAssetMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CAssetMesh::Free()
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