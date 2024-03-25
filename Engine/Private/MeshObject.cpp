#include "MeshObject.h"

#include "AssetManager.h"
#include "DeviceResource.h"
#include "Device_Utils.h"
#include "DXRResource.h"
#include "MeshData.h"

CMeshObject::CMeshObject(CMeshObject& rhs) : CComponent(rhs)
{
}


CMeshObject* CMeshObject::Create()
{
	CMeshObject* pInstance = new CMeshObject;
	pInstance->m_bIsPrototype = true;
	pInstance->Initialize_Prototype();

	return pInstance;
}

CComponent* CMeshObject::Clone(void* pArg)
{
	CMeshObject* pInstance = new CMeshObject(*this);

	pInstance->Initialize(pArg);

	return pInstance;
}

HRESULT CMeshObject::Initialize_Prototype()
{
	CComponent::Initialize_Prototype();

	return S_OK;
}

HRESULT CMeshObject::Initialize(void* pArg)
{
	CComponent::Initialize(pArg);

	MESHOBJ_INIT_DESC* init_desc = reinterpret_cast<MESHOBJ_INIT_DESC*>(pArg);

	if (init_desc->bIsSingle == true) // SingleÀº MeshData ÇÏ³ª¸¦ ¹Þ¾Æ¿È
	{
		CMeshData* pInstance = CAssetManager::Get_Instance()->Clone_MeshData(init_desc->strPrototypeTag, nullptr);

		if (pInstance)
		{
			m_vecMeshData.emplace_back(pInstance);
#if DXR_ON
			m_vecBlas.reserve(1);
			m_vecBlas.emplace_back(pInstance->Get_BLAS());
#endif
		}
	}
	else // clustered meshes, MeshDataList¸¦ ¹Þ¾Æ¿È
	{
		list<CMeshData*> meshList = CAssetManager::Get_Instance()->Clone_MeshData_Clustered(init_desc->strPrototypeTag);
		for (auto& iterMesh : meshList)
		{
			m_vecMeshData.emplace_back(std::move(iterMesh));
#if DXR_ON
			m_vecBlas.reserve(meshList.size());
			m_vecBlas.emplace_back(iterMesh->Get_BLAS());
#endif
			//m_vecAS_CPU.emplace_back(iter->Get_refAS_CPU()); // build as cpu
		}
	}

	if (m_vecMeshData.empty() == false)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMeshObject::Free()
{
	for (auto& iter : m_vecMeshData)
	{
		Safe_Release(iter);
	}
	m_vecMeshData.clear();

	return CComponent::Free();
}

D3D12_VERTEX_BUFFER_VIEW* CMeshObject::Get_VertexBufferViewPtr(UINT iMeshIndex)
{
	return m_vecMeshData[iMeshIndex]->Get_VertexBufferViewPtr();
}

D3D12_INDEX_BUFFER_VIEW* CMeshObject::Get_IndexBufferViewPtr(UINT iMeshIndex)
{
	return m_vecMeshData[iMeshIndex]->Get_IndexBufferViewPtr();
}

UINT CMeshObject::Get_CbvSrvUavOffset(UINT iMeshIndex)
{
	return m_vecMeshData[iMeshIndex]->Get_CbvSrvUavOffset();
}

void CMeshObject::Add_MeshData(CMeshData* pMeshData)
{
	m_vecMeshData.push_back(pMeshData);
}

#if DXR_ON
//DXR::ACCELERATION_STRUCTURE_CPU CMeshObject::Create_AccelerationStructures_CPU(CMeshData* pMeshData)
//{
//	ID3D12Device* pDevice = CDeviceResource::Get_Instance()->Get_Device();
//	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = CDXRResource::Get_Instance()->Get_refHeapHandle_CPU();
//	UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();
//
//	DXR::ACCELERATION_STRUCTURE_CPU as_cpu = 
//		::Build_AccelerationStructures_CPU(pMeshData, pDevice, cpuHandle, iDescriptorSize);
//
//	return as_cpu;
//}

HRESULT CMeshObject::Build_AccelerationStructures()
{
	HRESULT hr = S_OK;

	//AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count];
	//array<vector<D3D12_RAYTRACING_GEOMETRY_DESC>, 1+1/*Triangle+AABB*/> geometryDescs; // »ï°¢Çü°ú AABB½ÖÀÌ
	//pair<vector<D3D12_RAYTRACING_GEOMETRY_DESC>, D3D12_RAYTRACING_GEOMETRY_DESC>; // 
	//{
	//	D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	//	// Triangle geometry desc
	//	{
	//		// Triangle bottom-level AS contains a single plane geometry.
	//		geometryDescs[0].resize(1);

	//		// Plane geometry
	//		auto& geometryDesc = geometryDescs[BottomLevelASType::Triangle][0];
	//		geometryDesc = {};
	//		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	//		geometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
	//		geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexBuffer.resource->GetDesc().Width) / sizeof(Index);
	//		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
	//		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	//		geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
	//		geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
	//		geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
	//		geometryDesc.Flags = geometryFlags;
	//	}

	//	// AABB geometry desc
	//	{
	//		D3D12_RAYTRACING_GEOMETRY_DESC aabbDescTemplate = {};
	//		aabbDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
	//		aabbDescTemplate.AABBs.AABBCount = 1;
	//		aabbDescTemplate.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
	//		aabbDescTemplate.Flags = geometryFlags;

	//		// One AABB primitive per geometry.
	//		geometryDescs[BottomLevelASType::AABB].resize(IntersectionShaderType::TotalPrimitiveCount, aabbDescTemplate);

	//		// Create AABB geometries. 
	//		// Having separate geometries allows of separate shader record binding per geometry.
	//		// In this sample, this lets us specify custom hit groups per AABB geometry.
	//		for (UINT i = 0; i < IntersectionShaderType::TotalPrimitiveCount; i++)
	//		{
	//			auto& geometryDesc = geometryDescs[BottomLevelASType::AABB][i];
	//			geometryDesc.AABBs.AABBs.StartAddress = m_aabbBuffer.resource->GetGPUVirtualAddress() + i * sizeof(D3D12_RAYTRACING_AABB);
	//		}
	//	}


	//	//----[

	//	// Build all bottom-level AS.
	//	for (UINT i = 0; i < BottomLevelASType::Count; i++)
	//	{
	//		bottomLevelAS[i] = BuildBottomLevelAS(geometryDescs[i]);
	//	}
	//}

	return hr;
}

HRESULT CMeshObject::Build_BLAS(DXR::ACCELERATION_STRUCTURE_CPU& refAS_CPU)
{
	HRESULT hr = S_OK;



	return hr;
}
#endif
