#include "pch.h"
#include "ClientLoader.h"
#include "LoadHelper.h"
#include "MeshData.h"
#include "AssetManager.h"

CClientLoader* CClientLoader::Create()
{
	CClientLoader* pInstance = new CClientLoader();

	return pInstance;
}

void CClientLoader::Load()
{
	m_pHelper = CLoadHelper::Get_Instance();
	Safe_AddRef(m_pHelper);

	Load_Textures();
	Load_3DModels();
}

void CClientLoader::Load_Textures()
{
	m_pHelper->StartSign();

	TEXTURE_LOAD_DESC load_Desc{};

	load_Desc.strPath = m_strTextureRootPath + L"checkboard.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_Checkboard");

	load_Desc.strPath = m_strTextureRootPath + L"ice.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_ice");

	load_Desc.strPath = m_strTextureRootPath + L"Cubemap\\NightSkybox.dds";
	load_Desc.bIsCubeMap = true;
	m_pHelper->Load_Texture(load_Desc, L"Texture_NightSkybox");

	m_pHelper->EndSign_Texture();
}

void CClientLoader::Load_3DModels()
{
	m_pHelper->StartSign();

	std::string strPath = m_str3DModelRootPath + "zelda\\";
	std::string strAssetName = "zeldaPosed001.fbx";
	std::wstring wstrPrototypeTag = L"zeldaPosed001";

	list<CMeshData*> meshList;
	m_pHelper->Load_3DModel(strPath, strAssetName, &meshList);


	CAssetManager::Get_Instance()->Add_MeshData_ClusteredPrototype(wstrPrototypeTag, meshList);
}

HRESULT CClientLoader::Free()
{
	Safe_Release(m_pHelper);
	return S_OK;
}
