#include "AssetManager.h"
#include "Cube.h"
#include "MeshData.h"
#include "Texture.h"
#include "AssetMesh.h"

IMPLEMENT_SINGLETON(CAssetManager)

HRESULT CAssetManager::Free()
{
	for (auto& pair : m_mapTextures)
	{
		Safe_Release(pair.second);
	}
	m_mapTextures.clear();

	for (auto& pair : m_mapMeshData)
	{
		Safe_Release(pair.second);
	}
	m_mapMeshData.clear();

	for (auto& pair : m_mapMeshData_Clustered)
	{
		for (CMeshData*& pMeshInstance : pair.second)
		{
			Safe_Release(pMeshInstance);
		}
		pair.second.clear();
	}
	m_mapMeshData_Clustered.clear();


	return S_OK;
}

HRESULT CAssetManager::Add_Texture(const wstring& strAssetName, CTexture* pTextureInstance)
{
	// �̹� �����Ѵٸ�
	if (FindandGet_Texture(strAssetName))
	{
		MSG_BOX("AssetManager : Already Exists");
		return E_FAIL;
	}

	m_mapTextures.emplace(strAssetName, pTextureInstance);

	return S_OK;
}

CTexture* CAssetManager::FindandGet_Texture(const wstring& strAssetName)
{
	auto iter = m_mapTextures.find(strAssetName);

	// �������� �ʴ´ٸ�
	if (iter == m_mapTextures.end())
	{
		return nullptr;
	}

	return iter->second;
}

HRESULT CAssetManager::Add_MeshDataPrototype(const wstring& strPrototypeTag, CMeshData* pMeshData)
{
	if (FindandGet_MeshData(strPrototypeTag))
	{
		MSG_BOX("AssetManager : Already Exists");
		return E_FAIL;
	}

	m_mapMeshData.emplace(strPrototypeTag, pMeshData);

	return S_OK;
}

CMeshData* CAssetManager::FindandGet_MeshData(const wstring& strPrototypeTag)
{
	auto iter = m_mapMeshData.find(strPrototypeTag);

	// �������� �ʴ´ٸ�
	if (iter == m_mapMeshData.end())
	{
		return nullptr;
	}

	return iter->second;
}

CMeshData* CAssetManager::Clone_MeshData(const wstring& strPrototypeTag, void* pArg)
{
	auto iter = m_mapMeshData.find(strPrototypeTag);

	// �������� �ʴ´ٸ�
	if (iter == m_mapMeshData.end())
	{
		return nullptr;
	}

	return iter->second->Clone(pArg);
}

HRESULT CAssetManager::Add_MeshData_ClusteredPrototype(const wstring& strPrototypeTag, list<CMeshData*> meshDataList)
{
	if (FindandGet_MeshData_Clustered(strPrototypeTag).empty() == false)
	{
		MSG_BOX("AssetManager : Already Exists");
		return E_FAIL;
	}

	m_mapMeshData_Clustered.emplace(strPrototypeTag, meshDataList);

	return S_OK;
}

list<CMeshData*>& CAssetManager::FindandGet_MeshData_Clustered(const wstring& strPrototypeTag)
{
	auto iter = m_mapMeshData_Clustered.find(strPrototypeTag);

	// �������� �ʴ´ٸ�
	if (iter == m_mapMeshData_Clustered.end())
	{
		list<CMeshData*> emptyList;
		return emptyList;
	}

	return iter->second;
}

list<CMeshData*> CAssetManager::Clone_MeshData_Clustered(const wstring& strPrototypeTag)
{
	auto iter = m_mapMeshData_Clustered.find(strPrototypeTag);

	// 못찾음
	if (iter == m_mapMeshData_Clustered.end())
	{
		list<CMeshData*> emptyList;
		return emptyList;
	}

	list<CMeshData*> clonedList;
	for (auto& pair : m_mapMeshData_Clustered)
	{
		for (CMeshData*& meshInstance : pair.second)
		{
			clonedList.emplace_back(dynamic_cast<CAssetMesh*>(meshInstance)->Clone());
		}
	}

	return clonedList;
}
