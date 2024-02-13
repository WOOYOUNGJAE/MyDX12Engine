#include "pch.h"
#include "ClientLoader.h"
#include "LoadHelper.h"

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
}

void CClientLoader::Load_Textures()
{
	TEXTURE_LOAD_DESC load_Desc{};

	load_Desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_Checkboard");

	load_Desc.strPath = L"..\\..\\Resources\\Textures\\ice.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_ice");

	m_pHelper->EndSign_Texture();
}

HRESULT CClientLoader::Free()
{
	Safe_Release(m_pHelper);
	return S_OK;
}
