#include "CubeMesh.h"
#include "Device_Utils.h"

CCubeMesh::CCubeMesh()
{
	m_iNumVertices = 6 * 4;
	m_iNumIndices = 6 * 6;
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT16);
}

CCubeMesh::CCubeMesh(CCubeMesh& rhs) : CMeshData(rhs)
//m_vertexData(rhs.m_vertexData)
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

CCubeMesh* CCubeMesh::Create()
{
	CCubeMesh* pInstance = new CCubeMesh();
	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init CubeMesh Prototype");
	}

	return pInstance;
}

CMeshData* CCubeMesh::Clone(void* pArg)
{
	CCubeMesh* pInstance = new CCubeMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("CubeMesh : Failed to Clone");
		return nullptr;
	}
	return pInstance;
}

HRESULT CCubeMesh::Initialize_Prototype()
{
	HRESULT hr = S_OK;

	hr = CMeshData::Initialize_Prototype();
	if (FAILED(hr)) { return E_FAIL; }


	VertexPositionNormalTexture tempVertices[]
	{
		// front
		VertexPositionNormalTexture(Vector3(-1, -1, -1), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, -1), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, -1), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, -1, -1), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f)),

		// back
		VertexPositionNormalTexture(Vector3(-1, -1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(1.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, -1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(0.0f, 1.0f)),

		// top
		VertexPositionNormalTexture(Vector3(-1, +1, -1), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, +1), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, +1), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, -1), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)),

		// bottom
		VertexPositionNormalTexture(Vector3(-1, -1, -1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(1.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, -1, +1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, -1, +1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, -1, -1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(0.0f, 1.0f)),

		// left
		VertexPositionNormalTexture(Vector3(-1, -1, +1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, +1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, -1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(-1, -1, -1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)),

		// right
		VertexPositionNormalTexture(Vector3(1, -1, -1), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(1, +1, -1), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(1, +1, +1), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(1, -1, +1), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)),
	};
	/*for (auto& iter : tempVertices)
	{
		iter.position = iter.position * 0.5f;
	}*/
	m_vecVertexData.reserve(24);
	m_vecVertexData.assign(tempVertices, tempVertices + _countof(tempVertices));

	UINT16 indicesData[]
	{
		// front
		0, 1, 2,
		0, 2, 3,

		// back
		7, 6, 5,
		7, 5, 4,

		// top
		8, 9, 10,
		8, 10, 11,

		// bottom
		13, 12, 15,
		13, 15, 14,

		// left
		16, 17, 18,
		16, 18, 19,

		// right
		20, 21, 22,
		20, 22, 23,
	};

	const UINT iVertexBufferSize = sizeof(VertexPositionNormalTexture) * m_iNumVertices;
	const UINT iIndexBufferSize = sizeof(UINT16) * m_iNumIndices;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indicesData, iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		indicesData, iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	CMeshData::Build_BLAS(indicesData, m_vecVertexData.data(),
		sizeof(UINT16)* _countof(indicesData), sizeof(VertexPositionNormalTexture)* m_iNumVertices);
#endif
	return hr;
}

HRESULT CCubeMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CCubeMesh::Free()
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
