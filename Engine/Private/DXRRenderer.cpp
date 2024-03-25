#include "DXRRenderer.h"
#include "DeviceResource.h"
#include "MeshData.h"
#include "AssetManager.h"
#include "./Shaders/Raytracing.hlsl.h"

#if DXR_ON

const wchar_t* CDXRRenderer::m_tszHitGroupName = L"MyHitGroup";
const wchar_t* CDXRRenderer::m_tszRaygenShaderName = L"MyRaygenShader";
const wchar_t* CDXRRenderer::m_tszClosestHitShaderName = L"MyClosestHitShader";
const wchar_t* CDXRRenderer::m_tszMissShaderName = L"MyMissShader";

CDXRRenderer* CDXRRenderer::Create()
{
	CDXRRenderer* pInstance = new CDXRRenderer;

	/*if (FAILED(pInstance->Initialize(ppDevice)))
	{
		MSG_BOX("DXR: Create Failed");
		Safe_Release(pInstance);
	}*/

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

	hr = Create_PSOs();
	if (FAILED(hr))
	{
		MSG_BOX("Create DXR PSOs Failed");
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
		CD3DX12_DESCRIPTOR_RANGE descriptorRange[2];
		descriptorRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 1/*for DXR*/); // output texture
		descriptorRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 1); // static index vertex buffer

		CD3DX12_ROOT_PARAMETER rootParameterArr[4];
		rootParameterArr[0].InitAsDescriptorTable(1, &descriptorRange[0]);
		rootParameterArr[1].InitAsShaderResourceView(0, 1);
		rootParameterArr[2].InitAsConstantBufferView(0, 1); // SceneConstant
		rootParameterArr[3].InitAsDescriptorTable(1, &descriptorRange[1]);

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
		rootParameterArr[0].InitAsConstants(SizeOfInUint32(DXR::OBJECT_CB), 1, 1);
		CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(_countof(rootParameterArr), rootParameterArr);
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
			IID_PPV_ARGS(&m_pRootSigArr[DXR_ROOTSIG_LOCAL]));
		if (FAILED(hr)) { return hr; }
	}


	return hr;
}

HRESULT CDXRRenderer::Create_PSOs()
{
	HRESULT hr = S_OK;
	// SubObject 생성
	CD3DX12_STATE_OBJECT_DESC psoDesc { D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	// DXIL library
	{
		// entry point 지정, shader는 subobject 로 생성하지 않고 라이브러리에 묶음
		CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib =
			psoDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		D3D12_SHADER_BYTECODE libDXIL = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, _countof(g_pRaytracing));
		lib->SetDXILLibrary(&libDXIL);
		// Define which shader exports to surface from the library.
		// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
		// In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
		{
			// 라이브러리에서 Export 정의 -> PSO에서는 Import[
			lib->DefineExport(m_tszRaygenShaderName);
			lib->DefineExport(m_tszClosestHitShaderName);
			lib->DefineExport(m_tszMissShaderName);
		}
	}

	// Triangle HitGroup
	{
		CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = psoDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(m_tszClosestHitShaderName);
		hitGroup->SetHitGroupExport(m_tszHitGroupName);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}

	// Shader config, Attribute에는 무게중심 좌표, Payload는 레이가 진행하면서 데이터 저장하는 공간
	// Defines the maximum sizes in bytes for the ray payload and attribute structure.
	{
		CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig =
			psoDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		UINT payloadSize = sizeof(Vector4);   // float4 color
		UINT attributeSize = sizeof(Vector2); // float2 barycentrics
		shaderConfig->Config(payloadSize, attributeSize);
	}

	// RootSignature SubObject - LOCAL | shader association
	{
		// ray gen shader을 위한 Local root signature
		CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* localRootSignature = psoDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(m_pRootSigArr[DXR_ROOTSIG_LOCAL]);
		// shader association
		// associaton 객체를 만들어서 루트시그니처 서브오브젝트를 바인딩, 그 후 AddExport로 실제 쉐이더 프로그램에 연결
		CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = psoDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(m_tszHitGroupName);
	}

	// RootSignature SubObject - GLOBAL
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	{
		CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature =
			psoDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(m_pRootSigArr[DXR_ROOTSIG_GLOBAL]);
	}

	// Pipeline Config - Defines the maximum TraceRay() recursion depth.
	{
		CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pipelineConfig = 
			psoDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		UINT iMaxRecursionDepth = 1; // ~ primary rays only. 
		pipelineConfig->Config(iMaxRecursionDepth);
	}

	hr = m_pDevice->CreateStateObject(psoDesc, IID_PPV_ARGS(&m_pDXR_PSO));
	if (FAILED(hr))
	{
		MSG_BOX("Failed to Create DXR PSO");
	}

	return hr;
}

HRESULT CDXRRenderer::Build_AccelerationStructures()
{
	//HRESULT hr = S_OK;

	//map<wstring, CMeshData*>& refMeshData = CAssetManager::Get_Instance()->Get_MeshDataMap();
	//map<wstring, list<CMeshData*>>& refMeshData_Clustered =
	//	CAssetManager::Get_Instance()->Get_MeshData_ClusteredMap();

	//// 메쉬마다 Vertex, Index 들을 SRV로
	//// Index Srv 만든 후 Vertex Srv 생성
	//for (auto& pair : refMeshData)
	//{
	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc.Buffer.NumElements = numElements;
	//	if (elementSize == 0)
	//	{
	//		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	//		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	//		srvDesc.Buffer.StructureByteStride = 0;
	//	}
	//	else
	//	{
	//		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	//		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//		srvDesc.Buffer.StructureByteStride = elementSize;
	//	}
	//	UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
	//	m_pDevice->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);

	//	m_pDevice;

	//	DXR::ACCELERATION_STRUCTURE_CPU as_CPU{};
	//}

	//return hr;
	return S_OK;
}
#endif