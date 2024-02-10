#include "pch.h"
#include "Loader_Client.h"
#include "ComponentManager.h"
#include "Texture.h"

CClientLoader* CClientLoader::Create()
{
	CClientLoader* pInstance = new CClientLoader();


	return pInstance;
}

void CClientLoader::Load(ID3D12Device* pDevice)
{
	m_pCompoManager = CComponentManager::Get_Instance();
	m_pDevice = pDevice;
	Safe_AddRef(m_pCompoManager);
	Safe_AddRef(m_pDevice);

	Load_Textures();
}

void CClientLoader::Load_Textures()
{
	ResourceUploadBatch resourceUpload(m_pDevice);
	resourceUpload.Begin();

	TEXTURE_INIT_DESC texture_init_desc{};
	texture_init_desc.bIsCubeMap = false;
	texture_init_desc.pDevice = m_pDevice;
	texture_init_desc.pResourceUpload = &resourceUpload;

	texture_init_desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";

	m_pCompoManager->Add_Prototype(L"Texture_Checkboard", CTexture::Create(&texture_init_desc));

	auto finish = resourceUpload.End(CGraphic_Device::Get_Instance()->Get_CommandQueue().Get());
	finish.wait();
}

HRESULT CClientLoader::Free()
{
	Safe_Release(m_pCompoManager);
	Safe_Release(m_pDevice);
	return S_OK;
}
