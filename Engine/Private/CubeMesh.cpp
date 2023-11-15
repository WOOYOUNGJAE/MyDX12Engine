#include "CubeMesh.h"
#include "Device_Utils.h"

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

	if (FAILED(Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("CubeMesh : Failed to Clone");
		return nullptr;
	}

	return pInstance;
}

HRESULT CCubeMesh::Initialize_Prototype()
{
	if (m_pDevice == nullptr || m_pCommandList == nullptr)
	{
		MSG_BOX("CubeMesh: Device Null");
		return E_FAIL;
	}

	if (FAILED(CMeshGeometry::Initialize_Prototype()))
	{
		return E_FAIL;
	}

	m_vertexData = new MY_VERTEX[8]
	{
		// TODO : Color TEMP
		MY_VERTEX({ _float3(-1.0f, -1.0f, -1.0f), _float4(Colors::White) }),
		MY_VERTEX({ _float3(-1.0f, +1.0f, -1.0f), _float4(Colors::Black) }),
		MY_VERTEX({ _float3(+1.0f, +1.0f, -1.0f), _float4(Colors::Red) }),
		MY_VERTEX({ _float3(+1.0f, -1.0f, -1.0f), _float4(Colors::Green) }),
		MY_VERTEX({ _float3(-1.0f, -1.0f, +1.0f), _float4(Colors::Blue) }),
		MY_VERTEX({ _float3(-1.0f, +1.0f, +1.0f), _float4(Colors::Yellow) }),
		MY_VERTEX({ _float3(+1.0f, +1.0f, +1.0f), _float4(Colors::Cyan) }),
		MY_VERTEX({ _float3(+1.0f, -1.0f, +1.0f), _float4(Colors::Magenta) })
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

	const _uint iVertexBufferSize = sizeof(MY_VERTEX) * 8;
	const _uint iIndexBufferSize = sizeof(_ushort) * 8;

	if (FAILED(D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU)))
	{
		MSG_BOX("CubeMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vertexData, iVertexBufferSize);

	if (FAILED(D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU)))
	{
		MSG_BOX("CubeMesh : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indexData, iIndexBufferSize);

	if (FAILED(CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		m_vertexData, iVertexBufferSize, m_vertexBufferUploader, m_vertexBufferGPU)))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}

	if (FAILED(CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		indexData, iIndexBufferSize, m_indexBufferUploader, m_indexBufferGPU)))
	{
		MSG_BOX("CubeMesh : Failed to Create Buffer");
		return E_FAIL;
	}


	return S_OK;
}

HRESULT CCubeMesh::Initialize(void* pArg)
{
	return Initialize_Prototype();
}

HRESULT CCubeMesh::Free()
{
	if (FAILED(CMeshGeometry::Free()))
	{
		return E_FAIL;
	}

	Safe_Delete_Array(m_vertexData);

	return S_OK;
}
