#include "LoadHelper.h"
#include "Graphic_Device.h"
#include "ResourceUploadBatch.h"
#include "Texture.h"
#include "ComponentManager.h"

IMPLEMENT_SINGLETON(CLoadHelper)

HRESULT CLoadHelper::Initialize()
{
	m_pDevice = CGraphic_Device::Get_Instance()->Get_Device().Get();
	m_pComponentManager = CComponentManager::Get_Instance();

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pComponentManager);
	if (m_pDevice == nullptr)
	{
		MSG_BOX("LoadHelper: Device nullptr");
		return E_FAIL;
	}

	m_pResourceUpload = new ResourceUploadBatch(m_pDevice);

	m_texture_init_desc.pDevice = m_pDevice;
	m_texture_init_desc.pResourceUpload = m_pResourceUpload;

	return S_OK;
}

void CLoadHelper::LoadTextures_Begin()
{
	m_pResourceUpload->Begin();
}

HRESULT CLoadHelper::Load_Texture(const TEXTURE_LOAD_DESC& refTexture_load_desc, const wstring& strPrototypeTag)
{
	m_texture_init_desc.bIsCubeMap = refTexture_load_desc.bIsCubeMap;
	m_texture_init_desc.strPath = refTexture_load_desc.strPath;
	
	/*TEXTURE_INIT_DESC texture_init_desc{};
	texture_init_desc.bIsCubeMap = false;
	texture_init_desc.strPath = L"..\\..\\Resources\\Textures\\checkboard.dds";*/
	
	m_pComponentManager->Add_Prototype(strPrototypeTag, CTexture::Create(&m_texture_init_desc));
	auto finish = m_pResourceUpload->End(CGraphic_Device::Get_Instance()->Get_CommandQueue().Get());
	finish.wait();

	return S_OK;
}

HRESULT CLoadHelper::Free()
{
	Safe_Delete(m_pResourceUpload);

	Safe_Release(m_pComponentManager);
	Safe_Release(m_pDevice);

	return S_OK;
}

