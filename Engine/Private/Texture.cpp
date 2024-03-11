#include "Texture.h"
#include "Graphic_Device.h"
#include "D3DResourceManager.h"
#include "WICTextureLoader.h"

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

	if (m_strPath.rfind(L".dds") != wstring::npos) // dds 파일이라면
	{
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
	}
	else // dds 아닐 때
	{
		hr = CreateWICTextureFromFile(
			pInitDesc->pDevice,
			*pInitDesc->pResourceUpload,
			pInitDesc->strPath.c_str(),
			&m_pAssetData, false, 0);
		if (FAILED(hr))
		{
			MSG_BOX("Create not DDS Failed");
			return E_FAIL;
		}		
	}

	

	//CGraphic_Device::Get_Instance()->Get_CommandList()->ResourceBarrier(1,
	//	&CD3DX12_RESOURCE_BARRIER::Transition(m_pAssetData, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));


	m_bIsCubeMap = pInitDesc->bIsCubeMap;

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_pAssetData->GetDesc().Format;
	srvDesc.ViewDimension = m_bIsCubeMap ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(CGraphic_Device::Get_Instance()->Get_CbvSrvUavHeapStart_CPU());
	m_iCbvSrvUavHeapOffset = pInitDesc->iCbvSrvUavHeapOffset;
	handle.Offset(1, m_iCbvSrvUavHeapOffset);

	pInitDesc->pDevice->CreateShaderResourceView(
		m_pAssetData,
		&srvDesc,
		handle
		);

	//CD3DResourceManager::Get_Instance()->Register_Resource(MANAGED_RESOURCE_TEX, &m_pAssetData);


	return hr;
}
