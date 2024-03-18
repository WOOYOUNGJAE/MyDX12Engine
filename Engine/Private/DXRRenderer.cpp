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

	m_pDevice = CDeviceResource::Get_Instance()->Get_Device5();
	Safe_AddRef(m_pDevice);

	hr = m_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pCommandAllocator));
	if (FAILED(hr)) { return E_FAIL; }

	// CommandList
	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr,
		IID_PPV_ARGS(&m_pDxrCommandList));
	if (FAILED(hr))
	{
		MSG_BOX("DXR: CommandList QueryInterface Failed");
		return hr;		
	}

	// 지원 여부 확인
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 caps{};
	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &caps, sizeof(caps));
	if (FAILED(hr) || caps.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		MSG_BOX("DXR: Created Device5 Doesn't Support DXR");
		return hr;
	}

	// RootSig
	hr = Crete_RootSignatures();
	if (FAILED(hr))
	{
		MSG_BOX("Create DXR RootSigatures Failed");
		return hr;
	}


	return hr;
}

HRESULT CDXRRenderer::Free()
{
	Safe_Release(m_pDevice);
	return S_OK;
}

HRESULT CDXRRenderer::Crete_RootSignatures()
{
	HRESULT hr = S_OK;

	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	{
		CD3DX12_DESCRIPTOR_RANGE UAVDescriptor;
		UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 1/*for DXR*/);
		CD3DX12_ROOT_PARAMETER rootParameterArr[2];
		rootParameterArr[0].InitAsDescriptorTable(1, &UAVDescriptor);
		rootParameterArr[1].InitAsShaderResourceView(0);
		CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(_countof(rootParameterArr), rootParameterArr);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		hr = D3D12SerializeRootSignature(
			&globalRootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signature,
			&error);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)error->GetBufferPointer());
			return hr;
		}

		hr = m_pDevice->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_pRootSigArr[DXR_ROOTSIG_GLOBAL]));
		if (FAILED(hr)) { return hr; }

	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	{
		CD3DX12_ROOT_PARAMETER rootParameterArr[1]{};
		ZeroMemory(rootParameterArr, sizeof(CD3DX12_ROOT_PARAMETER));
		/*rootParameterArr[0].InitAsConstants(SizeOfInUint32(sizeof(m_rayGenCB), 0, 0));*/
		CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(0, rootParameterArr);
		localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		hr = D3D12SerializeRootSignature(
			&localRootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signature,
			&error);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)error->GetBufferPointer());
			return hr;
		}

		hr = m_pDevice->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_pRootSigArr[DXR_ROOTSIG_GLOBAL]));
		if (FAILED(hr)) { return hr; }
	}


	return hr;
}

HRESULT CDXRRenderer::Create_PSOs()
{
	HRESULT hr = S_OK;

	CD3DX12_STATE_OBJECT_DESC psoDesc { D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = psoDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	//D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));

	return hr;
}
