#include "SphereMesh.h"

#include "Device_Utils.h"

CSphereMesh::CSphereMesh()
{
	m_iVertexByteStride = sizeof(VertexPositionNormalColorTexture);
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
}

CSphereMesh::CSphereMesh(CSphereMesh& rhs) : CMeshData(rhs),
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

CSphereMesh* CSphereMesh::Create(UINT iNumSlices, UINT iNumStacks)
{
	CSphereMesh* pInstance = new CSphereMesh();
	pInstance->m_iNumSlices = iNumSlices;
	pInstance->m_iNumStacks = iNumStacks;

	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init CSphereMesh Prototype");
	}

	return pInstance;
}

CMeshData* CSphereMesh::Clone(void* pArg)
{
	CSphereMesh* pInstance = new CSphereMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("SphereMesh : Failed to Clone");
		return nullptr;
	}
	return pInstance;
}

HRESULT CSphereMesh::Initialize_Prototype()
{
	HRESULT hr = CMeshData::Initialize_Prototype();

#pragma region Vertex,Index
	const FLOAT dPhi = -XM_PI / FLOAT(m_iNumStacks); // 90도를 나누고 싶은 만큼 분할
	const FLOAT dTheta = -XM_2PI / FLOAT(m_iNumSlices);

	for (UINT i = 0; i <= m_iNumStacks; ++i)
	{
		// 맨 윗 점부터 회전으로 내려오면서 버텍스 생성
		Vector3 vStackStartPoint = Vector3::Transform(
			Vector3(0.f, -0.5f, 0.f),
			Matrix::CreateRotationZ(dPhi * (FLOAT)i));
		for (UINT j = 0; j <= m_iNumSlices; ++j)
		{
			VertexPositionNormalColorTexture v;

			v.position = Vector3::Transform(
				vStackStartPoint,
				Matrix::CreateRotationY(dTheta * (FLOAT)j));
			v.normal = v.position; // VectorZero에서 뻗는 방향
			XMStoreFloat3(&v.normal, XMVector3Normalize(XMLoadFloat3(&v.normal)));
			v.textureCoordinate = Vector2((FLOAT)j / m_iNumSlices, 1.f - (FLOAT)i / m_iNumStacks);

			m_vecVertexData.push_back(v);
		}
	}

	// Index
	vector<UINT16> vecIndex16Data;
	for (int i = 0; i < m_iNumStacks; ++i) {

		const int offset = (m_iNumSlices + 1) * i;

		for (int j = 0; j < m_iNumSlices; ++j) {

			vecIndex16Data.push_back(offset + j);
			vecIndex16Data.push_back(offset + j + m_iNumSlices + 1);
			vecIndex16Data.push_back(offset + j + 1 + m_iNumSlices + 1);

			vecIndex16Data.push_back(offset + j);
			vecIndex16Data.push_back(offset + j + 1 + m_iNumSlices + 1);
			vecIndex16Data.push_back(offset + j + 1);
		}
	}

	m_iNumVertices = m_vecVertexData.size();
	m_iNumIndices = vecIndex16Data.size();
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT16);


#pragma endregion


	const UINT iVertexBufferSize = sizeof(VertexPositionNormalColorTexture) * m_iNumVertices;
	const UINT iIndexBufferSize = sizeof(UINT16) * m_iNumIndices;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("SphereMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("SphereMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), vecIndex16Data.data(), iIndexBufferSize);

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("SphereMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		vecIndex16Data.data(), iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("SphereMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	CMeshData::Build_BLAS(iIndexBufferSize, sizeof(VertexPositionNormalColorTexture) * m_iNumVertices);
#endif
	return hr;
}

HRESULT CSphereMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CSphereMesh::Free()
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

