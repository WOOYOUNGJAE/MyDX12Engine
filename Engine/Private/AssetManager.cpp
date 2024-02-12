#include "AssetManager.h"
#include "Texture.h"

IMPLEMENT_SINGLETON(CAssetManager)

HRESULT CAssetManager::Free()
{
	for (auto& pair : m_mapTextures)
	{
		Safe_Release(pair.second);
	}
	m_mapTextures.clear();

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
