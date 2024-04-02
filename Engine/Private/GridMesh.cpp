#include "GridMesh.h"

#include "Device_Utils.h"

CGridMesh::CGridMesh()
{
	m_iVertexByteStride = sizeof(VertexPositionNormalTexture);
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
}

CGridMesh::CGridMesh(CGridMesh& rhs) : CMeshData(rhs),
m_iNumSlices(rhs.m_iNumSlices),
m_iNumStacks(rhs.m_iNumStacks)
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

CGridMesh* CGridMesh::Create(UINT iNumSlices, UINT iNumStacks)
{
	CGridMesh* pInstance = new CGridMesh();
	pInstance->m_iNumSlices = iNumSlices;
	pInstance->m_iNumStacks = iNumStacks;

	pInstance->m_iNumVertices = (pInstance->m_iNumSlices + 1) * (pInstance->m_iNumStacks + 1);
	pInstance->m_iNumIndices = iNumSlices * iNumStacks * 3 * 2;
	pInstance->m_iVertexBufferByteSize = pInstance->m_iNumVertices * pInstance->m_iVertexByteStride;
	pInstance->m_iIndexBufferByteSize = pInstance->m_iNumIndices * sizeof(UINT16);

	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init CGridMesh Prototype");
	}

	return pInstance;
}

CMeshData* CGridMesh::Clone(void* pArg)
{
	CGridMesh* pInstance = new CGridMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("GridMesh : Failed to Clone");
		return nullptr;
	}
	return pInstance;
}

HRESULT CGridMesh::Initialize_Prototype()
{
	HRESULT hr = CMeshData::Initialize_Prototype();

#pragma region Vertex,Index
	const FLOAT dx = 1.f / (FLOAT)m_iNumSlices;
	const FLOAT dy = 1.f / (FLOAT)m_iNumStacks;
	const Vector3 vLeftBottom = Vector3(-0.5f, -0.5f, 0.f);

	// Vertex
	// 최하단(j) 를 쌓아올리는 방식
	for (UINT i = 0; i <= m_iNumStacks; ++i)
	{
		Vector3 vStackStartPoint = Vector3::Transform(
			vLeftBottom, Matrix::CreateTranslation(Vector3(0.f, dy * i, 0.f)));

		for (UINT j = 0; j <= m_iNumSlices; ++j)
		{
			VertexPositionNormalTexture v;

			v.position = Vector3::Transform(
				vStackStartPoint,
				Matrix::CreateTranslation(Vector3(dx * j, 0.f, 0.f)));

			v.normal = Vector3(0.f, 0.f, -1.f);

			v.textureCoordinate = Vector2(j / (FLOAT)m_iNumSlices, 1.f - i / (FLOAT)m_iNumStacks);

			m_vecVertexData.push_back(v);
		}
	}

	// index
	vector<UINT16> vecIndex16Data;
	for (UINT j = 0; j < m_iNumStacks; ++j) {
		const int offset = (m_iNumSlices + 1) * j;

		for (int i = 0; i < m_iNumSlices; i++) {

			// 첫번째 삼각형
			vecIndex16Data.push_back(offset + i);
			vecIndex16Data.push_back(offset + i + m_iNumSlices + 1);
			vecIndex16Data.push_back(offset + i + 1 + m_iNumSlices + 1);
			// 두 번째 삼각형
			vecIndex16Data.push_back(offset + i);
			vecIndex16Data.push_back(offset + i + 1 + m_iNumSlices + 1);
			vecIndex16Data.push_back(offset + i + 1);
		}
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
		MSG_BOX("GridMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), vecIndex16Data.data(), iIndexBufferSize);

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		vecIndex16Data.data(), iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("GridMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	CMeshData::Build_BLAS(vecIndex16Data.data(), m_vecVertexData.data(),
		iIndexBufferSize, sizeof(VertexPositionNormalTexture) * m_iNumVertices);
#endif
	return hr;
}

HRESULT CGridMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CGridMesh::Free()
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
