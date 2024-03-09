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

	m_pHelper->StartSign();

	Load_Textures();
	Load_3DModels();

	m_pHelper->EndSign();
}

void CClientLoader::Load_Textures()
{

	TEXTURE_LOAD_DESC load_Desc{};

	load_Desc.strPath = m_strTextureRootPath + L"checkboard.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_Checkboard");

	load_Desc.strPath = m_strTextureRootPath + L"ice.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_ice");

	load_Desc.strPath = m_strTextureRootPath + L"Cubemap\\NightSkybox.dds";
	load_Desc.bIsCubeMap = true;
	m_pHelper->Load_Texture(load_Desc, L"Texture_NightSkybox");

	load_Desc.strPath = m_strTextureRootPath + L"earth.jpg";
	load_Desc.bIsCubeMap = false;
	m_pHelper->Load_Texture(load_Desc, L"Texture_Earth");

	load_Desc.strPath = m_strTextureRootPath + L"Tree0.png";
	load_Desc.bIsCubeMap = false;
	m_pHelper->Load_Texture(load_Desc, L"Texture_Tree0");

}

void CClientLoader::Load_3DModels()
{

	std::string strPath = m_str3DModelRootPath + "zelda\\";
	std::string strAssetName = "zeldaPosed001.fbx";
	std::wstring wstrMeshPrototypeTag = L"zeldaPosed001";

	list<CMeshData*> meshList;
	m_pHelper->Load_3DModel(strPath, strAssetName, &meshList);
	//CMeshData::Normalize_Vertices(meshList);
	CAssetManager::Get_Instance()->Add_MeshData_ClusteredPrototype(wstrMeshPrototypeTag, meshList);


}

HRESULT CClientLoader::Free()
{
	Safe_Release(m_pHelper);
	return S_OK;
}
