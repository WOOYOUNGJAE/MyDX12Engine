#include "CubeMesh.h"
#include "Device_Utils.h"

CCubeMesh::CCubeMesh()
{
	m_iNumVertex = 8;
	m_iNumIndices = 6 * 6;
	m_iVertexByteStride = sizeof(VertexPositionColor);
	m_iVertexBufferByteSize = m_iNumVertex * m_iVertexByteStride;
	IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(_ushort);
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

	m_vertexData = new VertexPositionColor[8]
	{
		// TODO : Color TEMP
		VertexPositionColor({ _float3(-1.0f, -1.0f, -1.0f), _float4(Colors::White) }),
		VertexPositionColor({ _float3(-1.0f, +1.0f, -1.0f), _float4(Colors::Black) }),
		VertexPositionColor({ _float3(+1.0f, +1.0f, -1.0f), _float4(Colors::Red) }),
		VertexPositionColor({ _float3(+1.0f, -1.0f, -1.0f), _float4(Colors::Green) }),
		VertexPositionColor({ _float3(-1.0f, -1.0f, +1.0f), _float4(Colors::Blue) }),
		VertexPositionColor({ _float3(-1.0f, +1.0f, +1.0f), _float4(Colors::Yellow) }),
		VertexPositionColor({ _float3(+1.0f, +1.0f, +1.0f), _float4(Colors::Cyan) }),
		VertexPositionColor({ _float3(+1.0f, -1.0f, +1.0f), _float4(Colors::Magenta) })
	};

	_ushort indexData[36]
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const _uint iVertexBufferSize = sizeof(VertexPositionColor) * 8;
	const _uint iIndexBufferSize = sizeof(_ushort) * 8;

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
