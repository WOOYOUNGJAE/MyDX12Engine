#include "TriangleMesh.h"

#include "Device_Utils.h"

CTriangleMesh::CTriangleMesh()
{
	m_iNumVertex = 3;
	m_iNumIndices = 3;
	m_iVertexByteStride = sizeof(MY_VERTEX);
	m_iVertexBufferByteSize = m_iNumVertex * m_iVertexByteStride;
	IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(_ushort);
}

CTriangleMesh::CTriangleMesh(const CTriangleMesh& rhs) : CMeshGeometry(rhs)
{
	m_iNumVertex = rhs.m_iNumVertex;
	m_iNumIndices = rhs.m_iNumIndices;
	m_iVertexByteStride = rhs.m_iVertexByteStride;
	m_iVertexBufferByteSize = rhs.m_iVertexBufferByteSize;
	IndexFormat = rhs.IndexFormat;
	m_iIndexBufferByteSize = rhs.m_iIndexBufferByteSize;
}

CTriangleMesh* CTriangleMesh::Create()
{
	CTriangleMesh* pInstance = new CTriangleMesh();

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init TriangleMesh Prototype");
	}

	return pInstance;
}

CComponent* CTriangleMesh::Clone(void* pArg)
{
	CTriangleMesh* pInstance = new CTriangleMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("TriangleMesh : Failed to Clone");
		return nullptr;
	}
	pInstance->m_bIsCloned = true;
	return pInstance;
}

HRESULT CTriangleMesh::Initialize_Prototype()
{
	HRESULT hr = S_OK;
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("TriangleMesh: Device Null");
		return E_FAIL;
	}

	hr = CMeshGeometry::Initialize_Prototype();
	if (FAILED(hr)) { return E_FAIL; }

	m_vertexData = new MY_VERTEX[m_iNumVertex]
	{
		// TODO : Color TEMP
		MY_VERTEX({ _float3(0.f, 0.25f, 0.f), _float4(Colors::Red) }),
		MY_VERTEX({ _float3(0.25f, -0.25f, 0.0f), _float4(Colors::Green) }),
		MY_VERTEX({ _float3(-0.25f, -0.25f, 0.0f), _float4(Colors::Blue) }),
	};

	_ushort indexData[3]
	{
		0, 1, 2,
	};

	const _uint iVertexBufferSize = sizeof(MY_VERTEX) * 3;
	const _uint iIndexBufferSize = sizeof(_ushort) * 3;

	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vertexData, iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indexData, iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		m_vertexData, iVertexBufferSize, m_vertexBufferUploader, m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		indexData, iIndexBufferSize, m_indexBufferUploader, m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CTriangleMesh::Initialize(void* pArg)
{
	return CMeshGeometry::Initialize(pArg);
}

HRESULT CTriangleMesh::Free()
{
	if (FAILED(CMeshGeometry::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}
