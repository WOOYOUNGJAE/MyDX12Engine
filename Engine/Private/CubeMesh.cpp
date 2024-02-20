#include "CubeMesh.h"
#include "Device_Utils.h"

CCubeMesh::CCubeMesh()
{
	m_iNumVertex = 6 * 4;
	m_iNumIndices = 6 * 6;
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_iVertexBufferByteSize = m_iNumVertex * m_iVertexByteStride;
	IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(USHORT);
}

CCubeMesh::CCubeMesh(CCubeMesh& rhs) : CMeshGeometry(rhs),
m_vertexData(rhs.m_vertexData)
{
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

CComponent* CCubeMesh::Clone(void* pArg)
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

	hr = CMeshGeometry::Initialize_Prototype();
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
		VertexPositionNormalTexture(Vector3(+1, -1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, +1), Vector3(0.0f, 0.0f, 1.0f),  Vector2(1.0f, 0.0f)),

		// top
		VertexPositionNormalTexture(Vector3(-1, +1, -1), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, +1), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, +1), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(+1, +1, -1), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)),

		// bottom
		VertexPositionNormalTexture(Vector3(-1, -1, -1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(1.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(+1, -1, -1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(+1, -1, +1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(-1, -1, +1), Vector3(0.0f, -1.0f, 0.0f),  Vector2(1.0f, 0.0f)),

		// left
		VertexPositionNormalTexture(Vector3(-1, -1, +1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, +1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(-1, +1, -1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(-1, -1, -1), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)),

		// right
		VertexPositionNormalTexture(Vector3(1, -1, -1), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)),
		VertexPositionNormalTexture(Vector3(1, +1, -1), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(1, +1, +1), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)),
		VertexPositionNormalTexture(Vector3(1, -1, +1), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f))
	};

	UINT32 indexData[36]
	{
		// front
		0, 1, 2,
		0, 2, 3,

		// back
		4, 5, 6,
		4, 6, 7,

		// top
		8, 9, 10,
		8, 10, 11,

		// bottom
		12, 13, 14,
		12, 14, 15,

		// left
		16, 17, 18,
		16, 18, 19,

		// right
		20, 21, 22,
		20, 22, 23,
	};

	const UINT iVertexBufferSize = sizeof(VertexPositionNormalTexture) * m_iNumVertex;
	const UINT iIndexBufferSize = sizeof(UINT32) * m_iNumIndices;

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

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		m_vertexData, iVertexBufferSize, m_vertexUploadBuffer, m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		indexData, iIndexBufferSize, m_indexUploadBuffer, m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}


	return S_OK;
}

HRESULT CCubeMesh::Initialize(void* pArg)
{
	return CMeshGeometry::Initialize(pArg);
}

HRESULT CCubeMesh::Free()
{
	if (m_iClonedNum == 0)
	{
		Safe_Delete_Array(m_vertexData); // Prototype 경우에만 해제
	}
	if (FAILED(CMeshGeometry::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}
