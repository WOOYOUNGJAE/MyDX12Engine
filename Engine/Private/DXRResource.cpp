#include "DXRResource.h"

#include "AssetManager.h"
#include "Device_Utils.h"
#include "DeviceResource.h"
#include "DXRRenderer.h"
#include "MeshData.h"
#include "DXRShaderTable.h"
//#include "./Shaders/Raytracing.hlsl.h"
#include "D:\\Projects\\Git\\MyProjects\\MyDX12Engine\\Engine\\Bin\\Intermediate\\CompiledShaders\Raytracing.hlsl.h"

#if DXR_ON

IMPLEMENT_SINGLETON(CDXRResource)

const wchar_t* CDXRResource::m_tszHitGroupName = L"MyHitGroup";
const wchar_t* CDXRResource::m_tszRaygenShaderName = L"MyRaygenShader";
const wchar_t* CDXRResource::m_tszClosestHitShaderName = L"MyClosestHitShader";
const wchar_t* CDXRResource::m_tszMissShaderName = L"MyMissShader";

HRESULT CDXRResource::Initialize()
{
	HRESULT hr = S_OK;

	m_pDevice = CDeviceResource::Get_Instance()->Get_Device5();
	m_pCommandQueue = CDeviceResource::Get_Instance()->Get_CommandQueue();
	Safe_AddRef(m_pDevice);

	for (UINT i = 0; i < m_iBackBufferCount; ++i)
	{
		hr = m_pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&(m_pCommandAllocatorArr[i])));
		if (FAILED(hr)) { return E_FAIL; }
		m_pCommandAllocatorArr[i]->SetName(L"DXR_CommandAlloc");
	}

	// CommandList
	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocatorArr[0], nullptr,
		IID_PPV_ARGS(&m_pCommandList));
	if (FAILED(hr))
	{
		MSG_BOX("DXR: CommandList QueryInterface Failed");
		return hr;
	}
	// 닫힌 상태로 시작
	m_pCommandList->Close();

	// Fence Resource
	hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(hr))
	{
		MSG_BOX("Failed To Create Fence");
		return E_FAIL;
	}
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_queue_flush_desc = {
		&m_iFenceValue,
		m_pCommandQueue,
		m_pFence,
		&m_fenceEvent
	};

	// 지원 여부 확인
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 caps{};
	hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &caps, sizeof(caps));
	if (FAILED(hr) || caps.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
	{
		MSG_BOX("DXR: Created Device5 Doesn't Support DXR");
		return hr;
	}

	// DescriptorHeap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 4000/*DXR생성시엔 오브젝트 개수 모르기 때문에*/;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	m_pDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap));

	m_iDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_curHeapHandle_CPU = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

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

	hr = Build_ShaderTable();
	if (FAILED(hr))
	{
		MSG_BOX("Building Shader Table Failed");
		return hr;
	}

	hr = Create_OutputResource();
	if (FAILED(hr))
	{
		MSG_BOX("Creating Output Resource Failed");
		return hr;
	}

	return hr;
}

HRESULT CDXRResource::Crete_RootSignatures()
{
	HRESULT hr = S_OK;

	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
LOCAL_BLOCK_
	CD3DX12_DESCRIPTOR_RANGE descriptorRange[2];
	descriptorRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 1/*for DXR*/); // output texture
	descriptorRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 1); // static index vertex buffer

	CD3DX12_ROOT_PARAMETER rootParameterArr[4];
	rootParameterArr[GlobalRootSigSlot::RENDER_TARGET].InitAsDescriptorTable(1, &descriptorRange[0]);
	rootParameterArr[GlobalRootSigSlot::AS].InitAsShaderResourceView(0, 1);
	rootParameterArr[GlobalRootSigSlot::PASS_CONSTANT].InitAsConstantBufferView(0, 1); // SceneConstant
	rootParameterArr[GlobalRootSigSlot::IB_VB_SRV].InitAsDescriptorTable(1, &descriptorRange[1]);

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
_LOCAL_BLOCK

LOCAL_BLOCK_
	// Local RootSignature
	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[LocalRootSigSlot::OBJECT_CB_STATIC].InitAsConstants(SizeOfInUint32(DXR::OBJECT_CB_STATIC) * 10/*TODO 개수 TEMP*/, 1, 1);
	CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
	localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error;

	hr = D3D12SerializeRootSignature(
		&localRootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&blob, &error);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		return hr;
	}

	hr = m_pDevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSigArr[DXR_ROOTSIG_LOCAL]));
	if (FAILED(hr)) { return hr; }
_LOCAL_BLOCK

	return hr;
}

HRESULT CDXRResource::Create_PSOs()
{
	HRESULT hr = S_OK;
	// SubObject 생성
	CD3DX12_STATE_OBJECT_DESC psoDesc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

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
	// Payload, Attribute 사이즈 정의
	// Defines the maximum sizes in bytes for the ray payload and attribute structure.
	{
		CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig =
			psoDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		UINT payloadSize = sizeof(Vector4) + sizeof(UINT);   // float4 color
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

	// RootSignature SubObject - GLOBAL, 모든 Shader에서 공통으로 참조 가능
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
		UINT iMaxRecursionDepth = 2; // ~ primary rays only. 
		pipelineConfig->Config(iMaxRecursionDepth);
	}

	hr = m_pDevice->CreateStateObject(psoDesc, IID_PPV_ARGS(&m_pDXR_PSO));
	if (FAILED(hr))
	{
		MSG_BOX("Failed to Create DXR PSO");
	}

	return hr;
}

HRESULT CDXRResource::Build_ShaderTable()
{
	HRESULT hr = S_OK;

	void* pRayGenShaderIdentifier;
	void* pMissShaderIdentifier;
	void* pHitGroupShaderIdentifier;

	ID3D12StateObjectProperties* pStateObjectProperties = nullptr;
	hr = m_pDXR_PSO->QueryInterface(&pStateObjectProperties);
	if (FAILED(hr)) { return hr; }

	pRayGenShaderIdentifier = pStateObjectProperties->GetShaderIdentifier(m_tszRaygenShaderName);
	pMissShaderIdentifier = pStateObjectProperties->GetShaderIdentifier(m_tszMissShaderName);
	pHitGroupShaderIdentifier = pStateObjectProperties->GetShaderIdentifier(m_tszHitGroupName);

	if (pRayGenShaderIdentifier == nullptr || pMissShaderIdentifier == nullptr || pHitGroupShaderIdentifier == nullptr)
	{
		return E_FAIL;
	}

	// Get shader identifiers.
	UINT iShaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	UINT iNumShaderRecords = 1;
	UINT iSingleRecordSize = iShaderIdentifierSize;
LOCAL_BLOCK_// Ray Gen Shader Table
	using DXR::TABLE_RECORD_DESC;
	TABLE_RECORD_DESC tableRecordDesc = TABLE_RECORD_DESC
	{
		D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
		pRayGenShaderIdentifier,
		0,
		nullptr
	};

	iNumShaderRecords = 1;
	iSingleRecordSize = iShaderIdentifierSize;

	CDXRShaderTable* pTableInstance = CDXRShaderTable::Create(
		m_pDevice,
		iNumShaderRecords,
		iSingleRecordSize,
		L"RayGenShaderTable");

	// ShaderTable에 Record 등록
	pTableInstance->Register_Record(tableRecordDesc);

	m_pRayGenShaderTable = pTableInstance->Get_TableResource();
	Safe_Release(pTableInstance);
_LOCAL_BLOCK


LOCAL_BLOCK_// Miss Shader Table
	using DXR::TABLE_RECORD_DESC;
	TABLE_RECORD_DESC tableRecordDesc = TABLE_RECORD_DESC
	{
		D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
		pMissShaderIdentifier,
		0,
		nullptr
	};

	iNumShaderRecords = 1;
	iSingleRecordSize = iShaderIdentifierSize;

	CDXRShaderTable* pTableInstance = CDXRShaderTable::Create(
		m_pDevice,
		iNumShaderRecords,
		iSingleRecordSize,
		L"MissShaderTable");

	// ShaderTable에 Record 등록
	pTableInstance->Register_Record(tableRecordDesc);

	m_pMissShaderTable = pTableInstance->Get_TableResource();
	Safe_Release(pTableInstance);
_LOCAL_BLOCK


LOCAL_BLOCK_// Hit Group Shader Table
	using DXR::TABLE_RECORD_DESC;
	struct RootArguments
	{
		DXR::OBJECT_CB_STATIC cb[NUM_OBJECTS]{};
	}rootArguments;
	//rootArguments.cb = DXR::OBJECT_CB_STATIC{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

	TABLE_RECORD_DESC tableRecordDesc = TABLE_RECORD_DESC
	{
		D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
		pHitGroupShaderIdentifier,
		 (sizeof(DXR::OBJECT_CB_STATIC)),
		&rootArguments
	};

	iNumShaderRecords = 1;
	iSingleRecordSize = iShaderIdentifierSize + sizeof(RootArguments);

	CDXRShaderTable* pTableInstance = CDXRShaderTable::Create(
		m_pDevice,
		iNumShaderRecords,
		iSingleRecordSize,
		L"HitGroupShaderTable");

	// ShaderTable에 Record 등록
	pTableInstance->Register_Record(tableRecordDesc);

	m_pHitGroupShaderTable = pTableInstance->Get_TableResource();
	Safe_Release(pTableInstance);

_LOCAL_BLOCK

	Safe_Release(pStateObjectProperties);
	return hr;
}

HRESULT CDXRResource::Create_OutputResource()
{
	HRESULT hr = S_OK;

	m_iScreenWidth = CDeviceResource::Get_Instance()->m_iClientWinCX;
	m_iScreenHeight = CDeviceResource::Get_Instance()->m_iClientWinCY;

	// Create the output resource. The dimensions and format should match the swap-chain.
	CD3DX12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		CDeviceResource::Get_Instance()->Get_BackBufferFormat(),
		m_iScreenWidth,
		m_iScreenHeight,
		1,
		1,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	hr = m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&uavDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&m_pDXROutput));
	if (FAILED(hr)) { return hr; }

	m_pDXROutput->SetName(L"m_pDXROutput");

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	m_pDevice->CreateUnorderedAccessView(
		m_pDXROutput,
		nullptr,
		&UAVDesc,
		m_curHeapHandle_CPU);

	INT iHeapOffsetGPU = INT(m_curHeapHandle_CPU.ptr - m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr);
	m_DXROutputHeapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
	m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),1, iHeapOffsetGPU);

	Apply_DescriptorHandleOffset();

	return hr;
}

HRESULT CDXRResource::Reset_CommandList()
{
	return m_pCommandList->Reset(m_pCommandAllocatorArr[0], nullptr);
}

HRESULT CDXRResource::Close_CommandList()
{
	return m_pCommandList->Close();
}

HRESULT CDXRResource::Execute_CommandList()
{
	ID3D12CommandList* pCommandListArr[] = { m_pCommandList, };
	m_pCommandQueue->ExecuteCommandLists(_countof(pCommandListArr), pCommandListArr);
	return S_OK;
}

void CDXRResource::AssignShaderIdentifiers(ID3D12StateObjectProperties* stateObjectProperties,
	void** ppRayGenShaderIdentifier, void** ppMissShaderIdentifier, void** ppHitGroupShaderIdentifier)
{
	*ppRayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(m_tszRaygenShaderName);
	*ppMissShaderIdentifier = stateObjectProperties->GetShaderIdentifier(m_tszMissShaderName);
	*ppHitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(m_tszHitGroupName);
}

void CDXRResource::Flush_CommandQueue()
{
	CDeviceResource::Get_Instance()->Flush_CommandQueue(&m_queue_flush_desc);
}

UINT64 CDXRResource::Apply_DescriptorHandleOffset()
{
	m_curHeapHandle_CPU.Offset(INT(m_iDescriptorSize));
	++m_iCurOffsetInDescriptors;

	return m_iCurOffsetInDescriptors;
}

HRESULT CDXRResource::Free()
{
	Flush_CommandQueue();

	Safe_Release(m_pDXROutput);

	Safe_Release(m_pRayGenShaderTable);
	Safe_Release(m_pMissShaderTable);
	Safe_Release(m_pHitGroupShaderTable);

	CloseHandle(m_fenceEvent);
	Safe_Release(m_pScratchBuffer);
	Safe_Release(m_pFence);

	Safe_Release(m_pDXR_PSO);
	for (UINT i = 0; i < DXR_ROOTSIG_TYPE_END; ++i)
	{
		Safe_Release(m_pRootSigArr[i]);
	}
	Safe_Release(m_pDescriptorHeap);
	Safe_Release(m_pCommandList);
	for (UINT i = 0; i < m_iBackBufferCount; ++i)
	{
		Safe_Release(m_pCommandAllocatorArr[i]);		
	}
	Safe_Delete_Array(m_pCommandAllocatorArr);
	Safe_Release(m_pDevice);
	return S_OK;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CDXRResource::Get_HeapHandleGPU(UINT64 iOffsetInDescriptors)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(Get_HeapHandleStart_GPU(), iOffsetInDescriptors, m_iDescriptorSize);
}

#endif
