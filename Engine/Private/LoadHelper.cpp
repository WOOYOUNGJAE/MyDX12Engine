#include "LoadHelper.h"
#include "Graphic_Device.h"
#include "ResourceUploadBatch.h"
#include "Texture.h"
#include "ComponentManager.h"

IMPLEMENT_SINGLETON(CLoadHelper)

HRESULT CLoadHelper::Initialize()
{
	m_pDevice = CGraphic_Device::Get_Instance()->Get_Device().Get();
	CComponentManager::Get_Instance();
	return S_OK;
}

void CLoadHelper::LoadTextures_Begin()
{
	m_pResourceUpload = new ResourceUploadBatch(m_pDevice);
	
}

HRESULT CLoadHelper::LoadTextures()
{
	/*ResourceUploadBatch resourceUpload(pDevice);
	resourceUpload.Begin();

	TEXTURE_INIT_DESC texture_init_desc{};
	texture_init_desc.bIsCubeMap = false;
	texture_init_desc.pDevice = pDevice;
	texture_init_desc.pResourceUpload = &resourceUpload;
	texture_init_desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";

	Add_Prototype(L"Texture_Checkboard", CTexture::Create(&texture_init_desc));
	auto finish = resourceUpload.End(CGraphic_Device::Get_Instance()->Get_CommandQueue().Get());
	finish.wait();*/
	return S_OK;
}

HRESULT CLoadHelper::Free()
{
	Safe_Delete(m_pResourceUpload);

	return S_OK;
}

