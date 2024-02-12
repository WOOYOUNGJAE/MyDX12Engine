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
