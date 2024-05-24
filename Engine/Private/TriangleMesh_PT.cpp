#include "TriangleMesh_PT.h"

#include "Device_Utils.h"
#include "DeviceResource.h"

CTriangleMesh_PT::CTriangleMesh_PT()
{
	m_iNumVertices = 3;
	m_iNumIndices = 3;
	m_iVertexByteStride = sizeof(VertexPositionNormalColorTexture);
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(UINT16);
}

CTriangleMesh_PT::CTriangleMesh_PT(CTriangleMesh_PT& rhs) : CMeshData(rhs)
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

	
	VertexPositionNormalColorTexture tempVertices[]
	{
		{Vector3(0.0f, 0.25f, 0.0f), Vector3(0.0f, 0.0f, 1.0f),Color(1,0,0,1), Vector2(0.5f, 0.0f)},
		{Vector3(-0.25f, -0.25f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Color(0,0,1,1),Vector2(0.0f, 1.0f)},
		{Vector3(0.25f, -0.25f, 0.0f), Vector3(0.0f, 0.0f, 1.0f),Color(0,1,0,1),Vector2(1.0f, 1.0f)}
	};
	/*{
		{Vector3(0.f, -1, 1.5f), Vector3(0.0f, 0.0f, -1.0f),Color(1,0,0,1),  Vector2(0.5f, 0.0f)},
		{Vector3(-1, 1, 1.5f), Vector3(0.0f, 0.0f, -1.0f),Color(0,1,0,1),Vector2(1.0f, 1.0f)},
		{Vector3(1, 1, 1.5f), Vector3(0.0f, 0.0f, -1.0f),Color(0,0,1,1),Vector2(0.0f, 1.0f)}
	};*/
	m_vecVertexData.reserve(3);
	m_vecVertexData.assign(tempVertices, tempVertices + _countof(tempVertices));

	UINT16 indicesData[3]
	{
		0, 1, 2,
	};

	const _uint iVertexBufferSize = sizeof(VertexPositionNormalColorTexture) * m_iNumVertices;
	const _uint iIndexBufferSize = sizeof(UINT16) * m_iNumIndices;


	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vecVertexData.data(), iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indicesData, iIndexBufferSize);

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
		m_vecVertexData.data(), iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = MyUtils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          indicesData, iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Buffer");
		return E_FAIL;
	}

	CMeshData::Init_VBV_IBV();
#if DXR_ON
	m_BLAS.eGeometryType = GEOMETRY_TYPE::TRIANGLE;
	m_BLAS.vecIndices.reserve(_countof(indicesData));
	for (UINT i = 0; i < m_iNumIndices; ++i)
	{
		m_BLAS.vecIndices.emplace_back(indicesData[i]);
	}
	m_BLAS.vecVertices.reserve(_countof(tempVertices));
	for (UINT i = 0; i < m_iNumVertices; ++i)
	{
		m_BLAS.vecVertices.emplace_back(tempVertices[i]);
	}

	DXR_Util::Build_BLAS(
		CDeviceResource::Get_Instance()->Get_Device5(),
		m_pCommandList,
		&m_BLAS,
		m_indexBufferGPU,
		m_vertexBufferGPU,
		m_IndexFormat,
		m_iNumIndices,
		m_iNumVertices,
		UINT64(Get_SingleVertexSize()));
	//CMeshData::Build_BLAS(iIndexBufferSize, sizeof(VertexPositionNormalColorTexture) * m_iNumVertices);
#endif
	return S_OK;
}

HRESULT CTriangleMesh_PT::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTriangleMesh_PT::Free()
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
