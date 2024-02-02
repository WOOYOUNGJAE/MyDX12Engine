#include "Texture.h"

#include "Graphic_Device.h"

CTexture::CTexture()
{
}

CTexture::CTexture(const CTexture& rhs)
{
}

CTexture* CTexture::Create(void* pArg)
{
	CTexture* pInstance = new CTexture();

	pInstance->Initialize_Prototype(static_cast<TEXTURE_INIT_DESC*>(pArg));

	return pInstance;
}

CComponent* CTexture::Clone(void* pArg)
{
	return nullptr;
}

HRESULT CTexture::Initialize_Prototype()
{
	return CComponent::Initialize_Prototype();
}

HRESULT CTexture::Initialize_Prototype(TEXTURE_INIT_DESC* pInitDesc)
{
	HRESULT hr = S_OK;

	m_strPath = pInitDesc->strPath;

	hr = CreateDDSTextureFromFile(
		pInitDesc->pDevice,
		*pInitDesc->pResourceUpload,
		pInitDesc->strPath.c_str(),
		&m_pTextureData, false, 0, nullptr,
		&pInitDesc->bIsCubeMap);

	m_bIsCubeMap = pInitDesc->bIsCubeMap;

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_pTextureData->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	pInitDesc->pDevice->CreateShaderResourceView(
		m_pTextureData,
		&srvDesc,
		CGraphic_Device::Get_Instance()->Get_SRVHeapStart());

	return hr;
}

HRESULT CTexture::Initialize(void* pArg)
{
	return CComponent::Initialize(pArg);
}

HRESULT CTexture::Free()
{
	Safe_Release(m_pTextureData);
	return CComponent::Free();
}
