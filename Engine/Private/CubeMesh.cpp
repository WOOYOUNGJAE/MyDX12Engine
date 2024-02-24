#include "CubeMesh.h"
#include "Device_Utils.h"

CCubeMesh::CCubeMesh()
{
	m_iNumVertex = 6 * 4;
	m_iNumIndices = 6 * 6;
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_iVertexBufferByteSize = m_iNumVertex * m_iVertexByteStride;
	IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT16);
}

CCubeMesh::CCubeMesh(CCubeMesh& rhs) : CMeshData(rhs),
m_vertexData(rhs.m_vertexData)
{
	Safe_AddRef(m_vertexBufferCPU);
	Safe_AddRef(m_indexBufferCPU);
	Safe_AddRef(m_vertexBufferGPU);
	Safe_AddRef(m_indexBufferGPU);
	Safe_AddRef(m_vertexUploadBuffer);
	Safe_AddRef(m_indexUploadBuffer);

	m_iNumVertex = rhs.m_iNumVertex;
	m_iNumIndices = rhs.m_iNumIndices;
	m_iVertexByteStride = rhs.m_iVertexByteStride;
	m_iVertexBufferByteSize = rhs.m_iVertexBufferByteSize;
	IndexFormat = rhs.IndexFormat;
	m_iIndexBufferByteSize = rhs.m_iIndexBufferByteSize;
}

CCubeMesh* CCubeMesh::Create()
{
	CCubeMesh* pInstance = new CCubeMesh();

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
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("CubeMesh: Device Null");
		return E_FAIL;
	}

	hr = CMeshData::Initialize_Prototype();
	if (FAILED(hr)) { return E_FAIL; }

	m_vertexData = new VertexPositionNormalTexture[24]
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

	UINT16 indexData[36]
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

	const UINT iVertexBufferSize = sizeof(VertexPositionNormalTexture) * m_iNumVertex;
	const UINT iIndexBufferSize = sizeof(UINT16) * m_iNumIndices;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vertexData, iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indexData, iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          m_vertexData, iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          indexData, iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
	return S_OK;
}

HRESULT CCubeMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CCubeMesh::Free()
{
	

	if (m_iClonedNum == 0)
	{
		Safe_Delete_Array(m_vertexData); // Prototype 경우에만 해제
	}
	else
	{
		Safe_Release(m_vertexBufferCPU);
		Safe_Release(m_indexBufferCPU);
		Safe_Release(m_vertexBufferGPU);
		Safe_Release(m_indexBufferGPU);
		Safe_Release(m_vertexUploadBuffer);
		Safe_Release(m_indexUploadBuffer);
	}

	if (FAILED(CMeshData::Free()))
	{
		return E_FAIL;
	}
	
	return S_OK;
}
