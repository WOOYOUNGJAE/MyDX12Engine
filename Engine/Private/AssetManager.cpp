#include "AssetManager.h"
#include "Cube.h"
#include "MeshGeometry.h"
#include "Texture.h"

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
	
	return S_OK;
}

HRESULT CAssetManager::Add_Texture(const wstring& strAssetName, CTexture* pTextureInstance)
{
	// 이미 존재한다면
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

	// 존재하지 않는다면
	if (iter == m_mapTextures.end())
	{
		return nullptr;
	}

	return iter->second;
}

HRESULT CAssetManager::Add_MeshDataPrototype(const wstring& strPrototypeTag, CMeshData* pMeshData)
{
	// 이미 존재한다면
	if (FindandGet_Texture(strPrototypeTag))
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

	// 존재하지 않는다면
	if (iter == m_mapMeshData.end())
	{
		return nullptr;
	}

	return iter->second;
}

CMeshData* CAssetManager::Clone_MeshData(const wstring& strPrototypeTag, void* pArg)
{
	auto iter = m_mapMeshData.find(strPrototypeTag);

	// 존재하지 않는다면
	if (iter == m_mapMeshData.end())
	{
		return nullptr;
	}

	return iter->second->Clone(pArg);
}
