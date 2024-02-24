#include "MeshObject.h"

#include "AssetManager.h"
#include "MeshGeometry.h"

CMeshObject::CMeshObject(CMeshObject& rhs) : CComponent(rhs)
{
}


CMeshObject* CMeshObject::Create()
{
	CMeshObject* pInstance = new CMeshObject;

	pInstance->Initialize_Prototype();

	return pInstance;
}

CComponent* CMeshObject::Clone(void* pArg)
{
	CMeshObject* pInstance = new CMeshObject(*this);

	pInstance->Initialize(pArg);

	return pInstance;
}

HRESULT CMeshObject::Initialize_Prototype()
{
	CComponent::Initialize_Prototype();

	return S_OK;
}

HRESULT CMeshObject::Initialize(void* pArg)
{
	CComponent::Initialize(pArg);

	std::vector<wstring>* pVecMeshTag = reinterpret_cast<std::vector<wstring>*>(pArg);

	for (auto& iter : *pVecMeshTag)
	{
		CMeshData* pInstance = CAssetManager::Get_Instance()->Clone_MeshData(iter, nullptr);

		if (pInstance)
		{
			m_vecMeshData.push_back(pInstance);
		}
	}

	if (m_vecMeshData.empty() == false)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMeshObject::Free()
{
	for (auto& iter : m_vecMeshData)
	{
		Safe_Release(iter);
	}
	m_vecMeshData.clear();

	return CComponent::Free();
}

D3D12_VERTEX_BUFFER_VIEW* CMeshObject::Get_VertexBufferViewPtr(UINT iMeshIndex)
{
	return m_vecMeshData[iMeshIndex]->Get_VertexBufferViewPtr();
}

D3D12_INDEX_BUFFER_VIEW* CMeshObject::Get_IndexBufferViewPtr(UINT iMeshIndex)
{
	return m_vecMeshData[iMeshIndex]->Get_IndexBufferViewPtr();
}

void CMeshObject::Add_MeshData(CMeshData* pMeshData)
{
	m_vecMeshData.push_back(pMeshData);
}
