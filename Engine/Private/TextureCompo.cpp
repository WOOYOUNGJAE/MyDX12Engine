#include "TextureCompo.h"
#include "Texture.h"
#include "AssetManager.h"
#include "Graphic_Device.h"

CTextureCompo::CTextureCompo()
{
}

CTextureCompo* CTextureCompo::Create()
{
	CTextureCompo* pInstance = new CTextureCompo;
	pInstance->m_bIsPrototype = true;

	return pInstance;
}

CComponent* CTextureCompo::Clone(void* pArg)
{
	CComponent* pInstnace = new CTextureCompo(*this);

	HRESULT hr = pInstnace->Initialize(pArg);
	if (FAILED(hr))
	{
		MSG_BOX("Texture : Clone Failed");
		return nullptr;
	}

	return pInstnace;
}

HRESULT CTextureCompo::Initialize(void* pArg)
{
	m_strTextureName = *reinterpret_cast<wstring*>(pArg);
	m_iCbvSrvUavHeapOffset = 
		CAssetManager::Get_Instance()->FindandGet_Texture(m_strTextureName)->m_iCbvSrvUavHeapOffset;
	return CComponent::Initialize(pArg);
}

HRESULT CTextureCompo::Free()
{
	return CComponent::Free();
}
