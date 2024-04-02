#include "CylinderMesh.h"

#include "Device_Utils.h"

CCylinderMesh::CCylinderMesh()
{
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
}

CCylinderMesh::CCylinderMesh(CCylinderMesh& rhs) : CMeshData(rhs),
m_iNumSlices(rhs.m_iNumSlices),
m_fTopRadius(rhs.m_fTopRadius),
m_fBottomRadius(rhs.m_fBottomRadius)
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

CCylinderMesh* CCylinderMesh::Create(UINT iNumSlices, FLOAT fTopRadius, FLOAT fBottomRadius)
{
	CCylinderMesh* pInstance = new CCylinderMesh();
	pInstance->m_iNumSlices = iNumSlices;
	pInstance->m_fTopRadius = fTopRadius;
	pInstance->m_fBottomRadius = fBottomRadius;

	pInstance->m_iNumVertices = (pInstance->m_iNumSlices + 1) * 2;
	pInstance->m_iNumIndices = iNumSlices * 3 * 2;
	pInstance->m_iVertexBufferByteSize = pInstance->m_iNumVertices * pInstance->m_iVertexByteStride;
	pInstance->m_iIndexBufferByteSize = pInstance->m_iNumIndices * sizeof(UINT16);

	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init CCylinderMesh Prototype");
	}

	return pInstance;
}

CMeshData* CCylinderMesh::Clone(void* pArg)
{
	CCylinderMesh* pInstance = new CCylinderMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("CylinderMesh : Failed to Clone");
		return nullptr;
	}
	return pInstance;
}

HRESULT CCylinderMesh::Initialize_Prototype()
{
	HRESULT hr = CMeshData::Initialize_Prototype();

#pragma region Vertex,Index
	const FLOAT dTheta = -XM_2PI / FLOAT(m_iNumSlices);

	// 밑면 구성하는 버텍스
	for (UINT i = 0; i <= m_iNumSlices; ++i)
	{
		VertexPositionNormalTexture v;
		v.position = Vector3::Transform(
			Vector3(m_fBottomRadius, -0.5f, 0.f),
			Matrix::CreateRotationY(dTheta * FLOAT(i)));

		v.normal = v.position - Vector3(0.f, -0.5f, 0.f);
		XMStoreFloat3(&v.normal, XMVector3Normalize(XMLoadFloat3(&v.normal)));
		v.textureCoordinate = Vector2(FLOAT(i) / m_iNumSlices, 1.f);

		m_vecVertexData.push_back(v);
	}

	// 윗면
	for (UINT i = 0; i <= m_iNumSlices; ++i)
	{
		VertexPositionNormalTexture v;
		v.position = Vector3::Transform(
			Vector3(m_fBottomRadius, 0.5f, 0.f),
			Matrix::CreateRotationY(dTheta * FLOAT(i)));

		v.normal = v.position - Vector3(0.f, 0.5f, 0.f);
		XMStoreFloat3(&v.normal, XMVector3Normalize(XMLoadFloat3(&v.normal)));
		v.textureCoordinate = Vector2(FLOAT(i) / m_iNumSlices, 0.f);

		m_vecVertexData.push_back(v);
	}

	// Index
	vector<UINT16> vecIndex16Data;
	for (UINT i = 0; i < m_iNumSlices; ++i)
	{
		vecIndex16Data.push_back(i);
		vecIndex16Data.push_back(i + m_iNumSlices + 1);
		vecIndex16Data.push_back(i + 1 + m_iNumSlices + 1);

		vecIndex16Data.push_back(i);
		vecIndex16Data.push_back(i + 1 + m_iNumSlices + 1);
		vecIndex16Data.push_back(i + 1);
	}

	if (m_iNumVertices != m_vecVertexData.size() || m_iNumIndices != vecIndex16Data.size())
	{
		return E_FAIL;
	}
#pragma endregion


	const UINT iVertexBufferSize = sizeof(VertexPositionNormalTexture) * m_iNumVertices;
	const UINT iIndexBufferSize = sizeof(UINT16) * m_iNumIndices;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("CylinderMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("CylinderMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), vecIndex16Data.data(), iIndexBufferSize);

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CylinderMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		vecIndex16Data.data(), iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("CylinderMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	CMeshData::Build_BLAS(vecIndex16Data.data(), m_vecVertexData.data(),
		iIndexBufferSize, sizeof(VertexPositionNormalTexture) * m_iNumVertices);
#endif
	return hr;
}

HRESULT CCylinderMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CCylinderMesh::Free()
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
