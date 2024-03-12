#include "TriangleMesh.h"

#include "Device_Utils.h"
#include "DeviceResource.h"

CTriangleMesh::CTriangleMesh()
{
	m_iNumVertices = 3;
	m_iNumIndices = 3;
	m_iVertexByteStride = sizeof(VertexPositionColor);
	m_iVertexBufferByteSize = m_iNumVertices * m_iVertexByteStride;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(_ushort);
}

CTriangleMesh::CTriangleMesh(CTriangleMesh& rhs) : CMeshData(rhs),
m_vertexData(rhs.m_vertexData)
{
	m_iNumVertices = rhs.m_iNumVertices;
	m_iNumIndices = rhs.m_iNumIndices;
	m_iVertexByteStride = rhs.m_iVertexByteStride;
	m_iVertexBufferByteSize = rhs.m_iVertexBufferByteSize;
	m_IndexFormat = rhs.m_IndexFormat;
	m_iIndexBufferByteSize = rhs.m_iIndexBufferByteSize;
}

CTriangleMesh* CTriangleMesh::Create()
{
	CTriangleMesh* pInstance = new CTriangleMesh();
	pInstance->m_bIsPrototype = true;
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init TriangleMesh Prototype");
	}

	return pInstance;
}

CMeshData* CTriangleMesh::Clone(void* pArg)
{
	CTriangleMesh* pInstance = new CTriangleMesh(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("TriangleMesh : Failed to Clone");
		return nullptr;
	}
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

	hr = CMeshData::Initialize_Prototype();
	if (FAILED(hr)) { return E_FAIL; }

	m_vertexData = new VertexPositionColor[m_iNumVertices]
	{
		// TODO : Color TEMP
		VertexPositionColor({ _float3(0.f, 0.25f * CDeviceResource::Get_Instance()->m_fAspectRatio, 0.25f), _float4(1.0f, 0.0f, 0.0f, 1.0f) }),
		VertexPositionColor({ _float3(0.25f, -0.25f * CDeviceResource::Get_Instance()->m_fAspectRatio, 0.25f), _float4(0.0f, 1.0f, 0.0f, 1.0f) }),
		VertexPositionColor({ _float3(-0.25f, -0.25f * CDeviceResource::Get_Instance()->m_fAspectRatio, 0.25f), _float4(0.0f, 0.0f, 1.0f, 1.0f) }),
	};

	_ushort indexData[3]
	{
		0, 1, 2,
	};

	const _uint iVertexBufferSize = sizeof(VertexPositionColor) * 3;
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

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          m_vertexData, iVertexBufferSize, &m_vertexUploadBuffer, &m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice, m_pCommandList,
	                                          indexData, iIndexBufferSize, &m_indexUploadBuffer, &m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CTriangleMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTriangleMesh::Free()
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
