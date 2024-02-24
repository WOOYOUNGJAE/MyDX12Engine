#include "LoadHelper.h"
#include "Graphic_Device.h"
#include "ResourceUploadBatch.h"
#include "AssetManager.h"
#include "ComponentManager.h"
#include "Renderer.h"
#include "Texture.h"
#include "D3DResourceManager.h"

IMPLEMENT_SINGLETON(CLoadHelper)

HRESULT CLoadHelper::Initialize()
{
	m_pDevice = CGraphic_Device::Get_Instance()->Get_Device();
	m_pAssetManager = CAssetManager::Get_Instance();

	m_pNextCbvSrvUavHeapOffset = CGraphic_Device::Get_Instance()->Get_NextCbvSrvUavHeapOffsetPtr();

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pAssetManager);
	if (m_pDevice == nullptr)
	{
		MSG_BOX("LoadHelper: Device nullptr");
		return E_FAIL;
	}

	m_pResourceUpload = new ResourceUploadBatch(m_pDevice);

	m_texture_init_desc.pDevice = m_pDevice;
	m_texture_init_desc.pResourceUpload = m_pResourceUpload;

	m_iCbvSrvUavDescriptorSize = CGraphic_Device::Get_Instance()->Get_CbvSrvUavDescriptorSize();

	return S_OK;
}

void CLoadHelper::StartSign()
{
	CD3DResourceManager::Get_Instance()->Set_SrvOffsetStart((*m_pNextCbvSrvUavHeapOffset));
	CGraphic_Device::Get_Instance()->Reset_CmdList();
}

HRESULT CLoadHelper::Load_Texture(const TEXTURE_LOAD_DESC& refTexture_load_desc, const wstring& strAssetName)
{
	m_pResourceUpload->Begin();

	m_texture_init_desc.bIsCubeMap = refTexture_load_desc.bIsCubeMap;
	m_texture_init_desc.strPath = refTexture_load_desc.strPath;
	m_texture_init_desc.iCbvSrvUavHeapOffset = *m_pNextCbvSrvUavHeapOffset;

	m_pAssetManager->Add_Texture(strAssetName, CTexture::Create(&m_texture_init_desc));
	auto finish = m_pResourceUpload->End(CGraphic_Device::Get_Instance()->Get_CommandQueue());
	finish.wait();

	(*m_pNextCbvSrvUavHeapOffset) += m_iCbvSrvUavDescriptorSize;

	return S_OK;
}

void CLoadHelper::EndSign_Texture()
{
	CGraphic_Device::Get_Instance()->Close_CmdList();
	dynamic_cast<CRenderer*>(CComponentManager::Get_Instance()->FindandGet_Prototype(L"Renderer"))->Build_FrameResource();
	CD3DResourceManager::Get_Instance()->Set_SrvOffsetEnd((*m_pNextCbvSrvUavHeapOffset));
}

HRESULT CLoadHelper::Load_3DModel(const wstring& strPath, const wstring& strAssetName)
{
	HRESULT hr = S_OK;



	return hr;
}

void CLoadHelper::EndSign_3DModel()
{
}

HRESULT CLoadHelper::Free()
{
	Safe_Delete(m_pResourceUpload);

	Safe_Release(m_pAssetManager);
	Safe_Release(m_pDevice);

	return S_OK;
}

