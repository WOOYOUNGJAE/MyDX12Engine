#include "TriangleMesh_PT.h"

#include "Device_Utils.h"
#include "Graphic_Device.h"

CTriangleMesh_PT::CTriangleMesh_PT()
{
	m_iNumVertex = 3;
	m_iNumIndices = 3;
	m_iVertexByteStride = sizeof(VertexPositionTexture);
	m_iVertexBufferByteSize = m_iNumVertex * m_iVertexByteStride;
	IndexFormat = DXGI_FORMAT_R16_UINT;
	m_iIndexBufferByteSize = m_iNumIndices * sizeof(_ushort);
}

CTriangleMesh_PT::CTriangleMesh_PT(const CTriangleMesh_PT& rhs) : CMeshGeometry(rhs),
m_vertexData(rhs.m_vertexData)
{
	m_iNumVertex = rhs.m_iNumVertex;
	m_iNumIndices = rhs.m_iNumIndices;
	m_iVertexByteStride = rhs.m_iVertexByteStride;
	m_iVertexBufferByteSize = rhs.m_iVertexBufferByteSize;
	IndexFormat = rhs.IndexFormat;
	m_iIndexBufferByteSize = rhs.m_iIndexBufferByteSize;
}

CTriangleMesh_PT* CTriangleMesh_PT::Create()
{
	CTriangleMesh_PT* pInstance = new CTriangleMesh_PT();

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Init TriangleMesh_PT Prototype");
	}

	return pInstance;
}

CComponent* CTriangleMesh_PT::Clone(void* pArg)
{
	CTriangleMesh_PT* pInstance = new CTriangleMesh_PT(*this);


	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("TriangleMesh_PT : Failed to Clone");
		return nullptr;
	}
	pInstance->m_bIsCloned = true;
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

	hr = CMeshGeometry::Initialize_Prototype();
	if (FAILED(hr)) { return E_FAIL; }

	m_vertexData = new VertexPositionTexture[]
	{
		{ _float3(0.0f, 0.25f * CGraphic_Device::Get_Instance()->m_fAspectRatio , 0.0f), _float2(0.5f, 0.0f)},
		{ _float3(0.25f, -0.25f * CGraphic_Device::Get_Instance()->m_fAspectRatio, 0.0f), _float2(1.0f, 1.0f)},
		{_float3(-0.25f, -0.25f * CGraphic_Device::Get_Instance()->m_fAspectRatio , 0.0f), _float2(0.0f, 1.0f)}
	};

	_ushort indexData[3]
	{
		0, 1, 2,
	};

	const _uint iVertexBufferSize = sizeof(VertexPositionTexture) * 3;
	const _uint iIndexBufferSize = sizeof(_ushort) * 3;


	hr = D3DCreateBlob(iVertexBufferSize, &m_vertexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_vertexBufferCPU->GetBufferPointer(), m_vertexData, iVertexBufferSize);

	hr = D3DCreateBlob(iIndexBufferSize, &m_indexBufferCPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Blob");
		return E_FAIL;
	}
	memcpy(m_indexBufferCPU->GetBufferPointer(), indexData, iIndexBufferSize);

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		m_vertexData, iVertexBufferSize, m_vertexBufferUploader, m_vertexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Buffer");
		return E_FAIL;
	}

	hr = CDevice_Utils::Create_Buffer_Default(m_pDevice.Get(), m_pCommandList.Get(),
		indexData, iIndexBufferSize, m_indexBufferUploader, m_indexBufferGPU);
	if (FAILED(hr))
	{
		MSG_BOX("TriangleMesh_PT : Failed to Create Buffer");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CTriangleMesh_PT::Initialize(void* pArg)
{
	return CMeshGeometry::Initialize(pArg);
}

HRESULT CTriangleMesh_PT::Free()
{
	if (m_bIsCloned == false)
	{
		Safe_Delete_Array(m_vertexData); // Prototype 경우에만 해제
	}

	if (FAILED(CMeshGeometry::Free()))
	{
		return E_FAIL;
	}

	return S_OK;
}
