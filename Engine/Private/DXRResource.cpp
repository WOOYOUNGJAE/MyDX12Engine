#include "DXRResource.h"

#include "AssetManager.h"
#include "Device_Utils.h"
#include "DeviceResource.h"
#include "DXRRenderer.h"
#include "MeshData.h"
#include "./Shaders/Raytracing.hlsl.h"

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

	hr = m_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pCommandAllocator));
	if (FAILED(hr)) { return E_FAIL; }

	// CommandList
	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr,
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
	// Allocate a heap for 3 descriptors:
	// 2 - vertex and index buffer SRVs
	// 1 - raytracing output texture SRV
	/*UINT iNumMeshes = CAssetManager::Get_Instance()->Get_MeshDataMap().size();
	auto& clusteredMeshesMap =  CAssetManager::Get_Instance()->Get_MeshData_ClusteredMap();
	for (auto& pair : clusteredMeshesMap)
	{
		for (auto& iterMeshList : pair.second)
		{
			iNumMeshes += iterMeshList->Num_Vertices();			
		}
	}*/
	descriptorHeapDesc.NumDescriptors = 4000;
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

	return hr;
}

HRESULT CDXRResource::Crete_RootSignatures()
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

HRESULT CDXRResource::Build_AccelerationStructures_CPU()
{
	HRESULT hr = S_OK;

	//map<wstring, CMeshData*>& refMeshData = CAssetManager::Get_Instance()->Get_MeshDataMap();
	//map<wstring, list<CMeshData*>>& refMeshData_Clustered =
	//	CAssetManager::Get_Instance()->Get_MeshData_ClusteredMap();

	//// 메쉬마다 Vertex, Index 들을 SRV로
	//// Index Srv 만든 후 Vertex Srv 생성
	//CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//for (auto& pair : refMeshData)
	//{
	//	CMeshData* pMeshData = pair.second;
	//	DXR::ACCELERATION_STRUCTURE_CPU as_CPU{};
	//	D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& refTriangles = as_CPU.dxrGeometryDesc.Triangles;
	//	as_CPU.dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	//	as_CPU.dxrGeometryDesc.Triangles.Transform3x4 = 0;
	//	as_CPU.dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE


	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//	// Create Index SRV
	//	srvDesc.Buffer.NumElements = pMeshData->Num_Indices() / sizeof(UINT32);
	//	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	//	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	//	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	//	cpuHandle.Offset(1, m_iDescriptorSize);
	//	m_pDevice->CreateShaderResourceView(as_CPU.srv_Index, &srvDesc, cpuHandle); // Index Srv

	//	refTriangles.IndexFormat = pMeshData->Get_IndexFormat();
	//	refTriangles.IndexCount = pMeshData->Num_Indices();
	//	refTriangles.IndexBuffer = as_CPU.srv_Index->GetGPUVirtualAddress();

	//	// Create Vertex SRV
	//	srvDesc.Buffer.NumElements = pMeshData->Num_Vertices();
	//	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	//	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//	srvDesc.Buffer.StructureByteStride = UINT(pMeshData->Get_SingleVertexSize());
	//	cpuHandle.Offset(1, m_iDescriptorSize);
	//	m_pDevice->CreateShaderResourceView(as_CPU.srv_Vertex, &srvDesc, cpuHandle); // Index Srv

	//	refTriangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	//	refTriangles.VertexCount = pMeshData->Num_Vertices();
	//	refTriangles.VertexBuffer.StartAddress = as_CPU.srv_Vertex->GetGPUVirtualAddress();
	//	refTriangles.VertexBuffer.StrideInBytes = UINT64(pMeshData->Get_SingleVertexSize());

	//	m_mapAS_CPU.emplace(pMeshData, as_CPU);
	//}

	//for (auto& pair : refMeshData_Clustered)
	//{
	//	for (CMeshData*& refMesh : pair.second)
	//	{
	//		DXR::ACCELERATION_STRUCTURE_CPU as_CPU{};
	//		D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& refTriangles = as_CPU.dxrGeometryDesc.Triangles;
	//		as_CPU.dxrGeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	//		as_CPU.dxrGeometryDesc.Triangles.Transform3x4 = 0;
	//		as_CPU.dxrGeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE; // 일단 OPAQUE

	//		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//		// Create Index SRV
	//		srvDesc.Buffer.NumElements = refMesh->Num_Indices() / sizeof(UINT32);
	//		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	//		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	//		srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	//		cpuHandle.Offset(1, m_iDescriptorSize);
	//		m_pDevice->CreateShaderResourceView(as_CPU.srv_Index, &srvDesc, cpuHandle); // Index Srv

	//		refTriangles.IndexFormat = refMesh->Get_IndexFormat();
	//		refTriangles.IndexCount = refMesh->Num_Indices();
	//		refTriangles.IndexBuffer = as_CPU.srv_Index->GetGPUVirtualAddress();

	//		// Create Vertex SRV
	//		srvDesc.Buffer.NumElements = refMesh->Num_Vertices();
	//		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	//		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//		srvDesc.Buffer.StructureByteStride = UINT(refMesh->Get_SingleVertexSize());
	//		cpuHandle.Offset(1, m_iDescriptorSize);
	//		m_pDevice->CreateShaderResourceView(as_CPU.srv_Vertex, &srvDesc, cpuHandle); // Index Srv

	//		refTriangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Fixed
	//		refTriangles.VertexCount = refMesh->Num_Vertices();
	//		refTriangles.VertexBuffer.StartAddress = as_CPU.srv_Vertex->GetGPUVirtualAddress();
	//		refTriangles.VertexBuffer.StrideInBytes = UINT64(refMesh->Get_SingleVertexSize());

	//		m_mapAS_CPU.emplace(refMesh, as_CPU);
	//	}
	//}

	return hr;
}

HRESULT CDXRResource::Build_AccelerationStructure_GPU(DXR::ACCELERATION_STRUCTURE_CPU& refAS_CPU)
{
	HRESULT hr = S_OK;

	m_pCommandList->Reset(m_pCommandAllocator, nullptr);

	// Get required sizes for an acceleration structure.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = 
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE; // 부수고 만들기 빠름

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BLASDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refBottomLevelInputs = BLASDesc.Inputs;
	refBottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refBottomLevelInputs.Flags = buildFlags;
	refBottomLevelInputs.NumDescs = 1;
	refBottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	refBottomLevelInputs.pGeometryDescs = &refAS_CPU.dxrGeometryDesc;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC TLASDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& refTopLevelInputs = TLASDesc.Inputs;
	refTopLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	refTopLevelInputs.Flags = buildFlags;
	refTopLevelInputs.NumDescs = 1;
	refTopLevelInputs.pGeometryDescs = nullptr; // Top Level에는 안들어감
	refTopLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {}; // 필요한 메모리 크기 쿼리
	m_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&refTopLevelInputs, &topLevelPrebuildInfo);
	if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0) { hr = E_FAIL; return hr; }

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	m_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&refBottomLevelInputs, &bottomLevelPrebuildInfo);
	if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes == 0) { hr = E_FAIL; return hr; }

	ComPtr<ID3D12Resource> scratchForBuildingProcess; // 빌드하는 동안 필요한 임시 메모리

	AllocateUAVBuffer(
		m_pDevice,
		max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes),
		&scratchForBuildingProcess,
		D3D12_RESOURCE_STATE_COMMON,
		L"scratchForBuildingProcess");

	//
	//{
	//	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	//	AllocateUAVBuffer(m_pDevice, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
	//	AllocateUAVBuffer(m_pDevice, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");
	//}

	//// Create an instance desc for the bottom-level acceleration structure.
	//ComPtr<ID3D12Resource> instanceDescs;
	//D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	//instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
	//instanceDesc.InstanceMask = 1;
	//instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	//AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");

	//// Bottom Level Acceleration Structure desc
	//{
	//	BLASDesc.ScratchAccelerationStructureData = scratchForBuildingProcess->GetGPUVirtualAddress();
	//	BLASDesc.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	//}

	//// Top Level Acceleration Structure desc
	//{
	//	TLASDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
	//	TLASDesc.ScratchAccelerationStructureData = scratchForBuildingProcess->GetGPUVirtualAddress();
	//	TLASDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	//}

	//auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
	//	{
	//		raytracingCommandList->BuildRaytracingAccelerationStructure(&BLASDesc, 0, nullptr);
	//		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get()));
	//		raytracingCommandList->BuildRaytracingAccelerationStructure(&TLASDesc, 0, nullptr);
	//	};

	//// Build acceleration structure.
	//BuildAccelerationStructure(m_dxrCommandList.Get());

	//// Kick off acceleration structure construction.
	//m_deviceResources->ExecuteCommandList();

	//// Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
	//m_deviceResources->WaitForGpu();

	return hr;
}

HRESULT CDXRResource::Reset_CommandList()
{
	return m_pCommandList->Reset(m_pCommandAllocator, nullptr);
}

HRESULT CDXRResource::Close_CommandList()
{
	return m_pCommandList->Close();
}

HRESULT CDXRResource::Execute_CommnadList()
{
	ID3D12CommandList* pCommandListArr[] = { m_pCommandList, };
	m_pCommandQueue->ExecuteCommandLists(_countof(pCommandListArr), pCommandListArr);
	return S_OK;
}

void CDXRResource::Flush_CommandQueue()
{
	CDeviceResource::Get_Instance()->Flush_CommandQueue(&m_queue_flush_desc);
}

HRESULT CDXRResource::Free()
{
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
	Safe_Release(m_pCommandAllocator);
	Safe_Release(m_pDevice);
	return S_OK;
}
#endif