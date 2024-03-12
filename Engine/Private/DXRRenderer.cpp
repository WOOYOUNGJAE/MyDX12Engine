#include "DXRRenderer.h"
#include "DeviceResource.h"

CDXRRenderer* CDXRRenderer::Create(ID3D12Device** ppDevice)
{
	CDXRRenderer* pInstance = new CDXRRenderer;

	if (FAILED(pInstance->Initialize(ppDevice)))
	{
		MSG_BOX("DXR: Create Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

HRESULT CDXRRenderer::Initialize(ID3D12Device** ppDevice)
{
	HRESULT hr = S_OK;	

	hr = (*ppDevice)->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pCommandAllocator));
	if (FAILED(hr)) { return E_FAIL; }

	hr = (*ppDevice)->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr,
		IID_PPV_ARGS(&m_pDxrCommandList));
	if (FAILED(hr))
	{
		MSG_BOX("DXR: CommandList QueryInterface Failed");
		return hr;		
	}

	// 지원 여부 확인
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 caps{};
	hr = (*ppDevice)->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &caps, sizeof(caps));
	if (FAILED(hr) || caps.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		MSG_BOX("DXR: Created Device5 Doesn't Support DXR");
		return hr;
	}

	return hr;
}

HRESULT CDXRRenderer::Free()
{
	return S_OK;
}
