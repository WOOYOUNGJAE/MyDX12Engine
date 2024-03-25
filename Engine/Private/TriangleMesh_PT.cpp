#include "TriangleMesh_PT.h"

#include "Device_Utils.h"
#include "DeviceResource.h"

CTriangleMesh_PT::CTriangleMesh_PT()
{
	m_iNumVertices = 3;
	m_iNumIndices = 3;
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT32);
}

CTriangleMesh_PT::CTriangleMesh_PT(CTriangleMesh_PT& rhs) : CMeshData(rhs),
m_vertexData(rhs.m_vertexData)
{
	m_iNumVertices = rhs.m_iNumVertices;
	m_iNumIndices = rhs.m_iNumIndices;
	m_iVertexByteStride = rhs.m_iVertexByteStride;
	m_iVertexBufferByteSize = rhs.m_iVertexBufferByteSize;
	m_IndexFormat = rhs.m_IndexFormat;
	m_iIndexBufferByteSize = rhs.m_iIndexBufferByteSize;
}

CTriangleMesh_PT* CTriangleMesh_PT::Create()
{
	CTriangleMesh_PT* pInstance = new CTriangleMesh_PT();
	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init TriangleMesh_PT Prototype");
	}

	return pInstance;
}

CMeshData* CTriangleMesh_PT::Clone(void* pArg)
{
	CTriangleMesh_PT* pInstance = new CTriangleMesh_PT(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("TriangleMesh_PT : Failed to Clone");
		return nullptr;
	}
	return pInstance;
}

HRESULT CTriangleMesh_PT::Initialize_Prototype()
{
	HRESULT hr = S_OK;
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("TriangleMesh_PT: Device Null");
		return E_FAIL;
	}

	hr = CMeshData::Initialize_Prototype();
	if (FAILED(hr)) { return E_FAIL; }

	m_vertexData = new VertexPositionNormalTexture[]
	{
		{Vector3(0.0f, 0.25f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.5f, 0.0f)},
		{Vector3(0.25f, -0.25f, 0.0f), Vector3(0.0f, 0.0f, -1.0f),Vector2(1.0f, 1.0f)},
		{Vector3(- 0.25f, -0.25f, 0.0f), Vector3(0.0f, 0.0f, -1.0f),Vector2(0.0f, 1.0f)}
	};
	/*m_vertexData = new VertexPositionTexture[]
	{
		{Vector3(0.0f, 0.25f, 0.0f), Vector2(0.5f, 0.0f)},
		{Vector3(0.25f, -0.25f, 0.0f), Vector2(1.0f, 1.0f)},
		{Vector3(- 0.25f, -0.25f, 0.0f), Vector2(0.0f, 1.0f)}
	};*/

	UINT32 indexData[3]
	{
		0, 1, 2,
	};

	const _uint iVertexBufferSize = sizeof(VertexPositionNormalTexture) * 3;
	const _uint iIndexBufferSize = sizeof(UINT32) * 3;


	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vertexData, iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indexData, iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          m_vertexData, iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          indexData, iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	CMeshData::Build_BLAS(m_vecIndexData.data(), m_vecVertexData.data(),
		iIndexBufferSize, sizeof(VertexPositionNormalTexture) * m_iNumVertices);
#endif
	return S_OK;
}

HRESULT CTriangleMesh_PT::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTriangleMesh_PT::Free()
{
	if (m_bIsPrototype == true)
	{
		Safe_Delete_Array(m_vertexData); // Prototype 경우에만 해제
	}

	if (FAILED(CMeshData::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}
