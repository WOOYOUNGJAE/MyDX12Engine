#include "Texture.h"
#include "Graphic_Device.h"

CTexture* CTexture::Create(void* pArg)
{
	CTexture* pInstance = new CTexture();

	pInstance->Initialize(pArg);

	return pInstance;
}

HRESULT CTexture::Initialize(void* pArg)
{
	TEXTURE_INIT_DESC* pInitDesc = reinterpret_cast<TEXTURE_INIT_DESC*>(pArg);
	HRESULT hr = S_OK;

	m_strPath = pInitDesc->strPath;

	hr = CreateDDSTextureFromFile(
		pInitDesc->pDevice,
		*pInitDesc->pResourceUpload,
		pInitDesc->strPath.c_str(),
		&m_pAssetData, false, 0, nullptr,
		&pInitDesc->bIsCubeMap);
	if (FAILED(hr))
	{
		MSG_BOX("Create DDS Failed");
		return E_FAIL;
	}

	m_iCbvSrvUavHeapOffset = pInitDesc->iCbvSrvUavHeapOffset;

	m_bIsCubeMap = pInitDesc->bIsCubeMap;

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_pAssetData->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(CGraphic_Device::Get_Instance()->Get_CbvSrvUavHeapStart_CPU());
	handle.Offset(1, m_iCbvSrvUavHeapOffset);

	pInitDesc->pDevice->CreateShaderResourceView(
		m_pAssetData,
		&srvDesc,
		handle
		);

	return hr;
}
