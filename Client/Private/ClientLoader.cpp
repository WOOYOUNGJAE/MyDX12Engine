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

	m_pHelper->LoadTextures_Begin();
	load_Desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";
	m_pHelper->Load_Texture(load_Desc, L"Texture_Checkboard");
	/*ResourceUploadBatch resourceUpload(m_pDevice);
	resourceUpload.Begin();

	TEXTURE_INIT_DESC texture_init_desc{};
	texture_init_desc.bIsCubeMap = false;
	texture_init_desc.pDevice = m_pDevice;
	texture_init_desc.pResourceUpload = &resourceUpload;

	texture_init_desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";

	m_pCompoManager->Add_Prototype(L"Texture_Checkboard", CTexture::Create(&texture_init_desc));

	auto finish = resourceUpload.End(CGraphic_Device::Get_Instance()->Get_CommandQueue().Get());
	finish.wait();*/
}

HRESULT CClientLoader::Free()
{
	Safe_Release(m_pHelper);
	return S_OK;
}
