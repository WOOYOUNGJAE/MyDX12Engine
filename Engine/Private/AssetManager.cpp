#include "AssetManager.h"
#include "Cube.h"
#include "MeshData.h"
#include "Texture.h"
#include "AssetMesh.h"
#include "Device_Utils.h"
#include "DXRResource.h"

IMPLEMENT_SINGLETON(CAssetManager)

HRESULT CAssetManager::Free()
{
	Safe_Release(m_pUploadBuffer_CombinedIndices);
	Safe_Release(m_pUploadBuffer_CombinedVertices);
	Safe_Release(m_pCombinedPrototypeMeshIndices);
	Safe_Release(m_pCombinedPrototypeMeshVertices);
	
	for (auto& pair : m_mapTextures)
	{
		Safe_Release(pair.second);
	}
	m_mapTextures.clear();

	for (auto& iterMeshData : m_pSingleMeshDataArr)
	{
		Safe_Release(iterMeshData);
	}

	for (auto& pair : m_mapMeshData_Clustered)
	{
		for (CMeshData*& pMeshInstance : pair.second)
		{
			Safe_Release(pMeshInstance);
		}
		pair.second.clear();
	}
	m_mapMeshData_Clustered.clear();


	return S_OK;
}

HRESULT CAssetManager::Add_Texture(const wstring& strAssetName, CTexture* pTextureInstance)
{
	// �̹� �����Ѵٸ�
	if (FindandGet_Texture(strAssetName))
	{
		MSG_BOX("AssetManager : Already Exists");
		return E_FAIL;
	}

	m_mapTextures.emplace(strAssetName, pTextureInstance);

	return S_OK;
}

CTexture* CAssetManager::FindandGet_Texture(const wstring& strAssetName)
{
	auto iter = m_mapTextures.find(strAssetName);

	// �������� �ʴ´ٸ�
	if (iter == m_mapTextures.end())
	{
		return nullptr;
	}

	return iter->second;
}

HRESULT CAssetManager::Add_MeshDataPrototype(GEOMETRY_TYPE eGeometryType, CMeshData* pMeshData)
{
	if (FindandGet_MeshData(eGeometryType))
	{
		MSG_BOX("AssetManager : Already Exists");
		return E_FAIL;
	}

	m_pSingleMeshDataArr[eGeometryType] = pMeshData;

	return S_OK;
}

CMeshData* CAssetManager::FindandGet_MeshData(GEOMETRY_TYPE eGeometryType)
{
	if (eGeometryType < 0 || eGeometryType >= GEOMETRY_TYPE_COUNT)
	{
		return nullptr;
	}
	return m_pSingleMeshDataArr[eGeometryType];
}

CMeshData* CAssetManager::Clone_MeshData(GEOMETRY_TYPE eGeometryType, void* pArg)
{
	if (eGeometryType < 0 || eGeometryType >= GEOMETRY_TYPE_COUNT)
	{
		return nullptr;
	}

	return m_pSingleMeshDataArr[eGeometryType]->Clone(pArg);
}

HRESULT CAssetManager::Add_MeshData_ClusteredPrototype(const wstring& strPrototypeTag, list<CMeshData*> meshDataList)
{
	if (FindandGet_MeshData_Clustered(strPrototypeTag).empty() == false)
	{
		MSG_BOX("AssetManager : Already Exists");
		return E_FAIL;
	}

	m_mapMeshData_Clustered.emplace(strPrototypeTag, meshDataList);

	return S_OK;
}

list<CMeshData*>& CAssetManager::FindandGet_MeshData_Clustered(const wstring& strPrototypeTag)
{
	auto iter = m_mapMeshData_Clustered.find(strPrototypeTag);

	// �������� �ʴ´ٸ�
	if (iter == m_mapMeshData_Clustered.end())
	{
		list<CMeshData*> emptyList;
		return emptyList;
	}

	return iter->second;
}

list<CMeshData*> CAssetManager::Clone_MeshData_Clustered(const wstring& strPrototypeTag)
{
	auto iter = m_mapMeshData_Clustered.find(strPrototypeTag);

	// 못찾음
	if (iter == m_mapMeshData_Clustered.end())
	{
		list<CMeshData*> emptyList;
		return emptyList;
	}

	list<CMeshData*> clonedList;
	for (auto& pair : m_mapMeshData_Clustered)
	{
		for (CMeshData*& meshInstance : pair.second)
		{
			clonedList.emplace_back(dynamic_cast<CAssetMesh*>(meshInstance)->Clone());
		}
	}

	return clonedList;
}
#if DXR_ON
void CAssetManager::Build_IB_VB_SRV_Serialized(ID3D12Device5* pDevice, ID3D12GraphicsCommandList* pCommandList, UINT iStructureByteStride)
{
	CDXRResource* pDxrResource = CDXRResource::Get_Instance();
	IB_VB_SRV_startOffsetInDescriptors = pDxrResource->Get_CurOffsetInDescriptors();
	UINT iNumAllIndices = 0;
	UINT iNumAllVertices = 0;

	std::vector<UINT16> vecAllIndices;
	auto iterIndicesDest = vecAllIndices.begin();
	UINT iStartIndex_in_SRV = 0;
	UINT iNumBlas = 3;
	for (UINT i = 0; i < iNumBlas/*TODO*/; ++i)
	{
		DXR::BLAS_INFOS& refBlasInfo = m_pSingleMeshDataArr[i]->Get_refBLAS();
		const auto& vecIndicesSrc = refBlasInfo.vecIndices;
		refBlasInfo.iStartIndex_in_IB_SRV = iStartIndex_in_SRV;

		for (UINT j = 0; j < vecIndicesSrc.size(); ++j)
		{
			vecAllIndices.emplace_back(vecIndicesSrc[j]);
		}
		INT iLackCount = 4 - (vecIndicesSrc.size() % 4);
		if (iLackCount == 4) // 딱 4바이트 단위로 맞아 떨어지면
		{
			iStartIndex_in_SRV = vecAllIndices.size();
			continue;
		}
		for (UINT j = 0; j < iLackCount; ++j)
		{
			vecAllIndices.emplace_back(0); // 3의 배수에 2개가 부족하면
		}
		iStartIndex_in_SRV = vecAllIndices.size();
	}
	vecAllIndices.shrink_to_fit();

	MyUtils::Create_Buffer_Default(pDevice, pCommandList, vecAllIndices.data(), sizeof(UINT16) * vecAllIndices.size(), &m_pUploadBuffer_CombinedIndices, &m_pCombinedPrototypeMeshIndices);
	iNumAllIndices = vecAllIndices.size();

	// VB
	iStartIndex_in_SRV = 0;
	for (UINT i = 0; i < iNumBlas; ++i)
	{
		iNumAllVertices += m_pSingleMeshDataArr[i]->Get_refBLAS().vecVertices.size();
	}
	VertexPositionNormalColorTexture* allVertices = new VertexPositionNormalColorTexture[iNumAllVertices];
	VertexPositionNormalColorTexture* pVerticesCopyDst = allVertices;
	for (UINT i = 0; i < iNumBlas; ++i)
	{
		DXR::BLAS_INFOS& refBlasInfo = m_pSingleMeshDataArr[i]->Get_refBLAS();
		refBlasInfo.iStartIndex_in_VB_SRV = iStartIndex_in_SRV;
		std::copy(refBlasInfo.vecVertices.begin(), refBlasInfo.vecVertices.end(), pVerticesCopyDst);
		UINT iCurVerticesSize = refBlasInfo.vecVertices.size();
		pVerticesCopyDst += iCurVerticesSize;
		iStartIndex_in_SRV += iCurVerticesSize;
	}
	MyUtils::Create_Buffer_Default(pDevice, pCommandList, allVertices, sizeof(VertexPositionNormalColorTexture) * iNumAllVertices, &m_pUploadBuffer_CombinedVertices, &m_pCombinedPrototypeMeshVertices);

	pDxrResource->Close_CommandList();
	pDxrResource->Execute_CommandList();
	pDxrResource->Flush_CommandQueue();
	pDxrResource->Reset_CommandList();


	// 모든 BLAS에 대한 Index SRV를 연속적으로 만든 후 Vertex SRV 만들기
	CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = pDxrResource->Get_refHeapHandle_CPU();
	UINT iDescriptorSize = CDXRResource::Get_Instance()->Get_DescriptorSize();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#pragma region Create Serial SRV of IB
	UINT iSingleIdexElementSize = sizeof(UINT16);

	// Create Index SRV
	srvDesc.Buffer.NumElements =
		(iSingleIdexElementSize * iNumAllIndices) / sizeof(UINT32); // 단순 인덱스 원소 개수가 아니라 UINT32로 얼마나 만들어지는지, D3D12_BUFFER_SRV_FLAG_RAW이라서
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS; // for Index Srv
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW; // 인덱스는 단순 정수 나열이므로 raw타입으로
	srvDesc.Buffer.StructureByteStride = 0; //  D3D12_BUFFER_SRV_FLAG_RAW, 즉 원시 데이터로 접근할 때
	//cpuHandle.Offset(1, iDescriptorSize);
	pDevice->CreateShaderResourceView(m_pCombinedPrototypeMeshIndices, &srvDesc, cpuHandle); // Index Srv
	pDxrResource->Apply_DescriptorHandleOffset();
#pragma endregion Create Serial SRV of IB
#pragma region Create Serial SRV of VB
	// Create Vertex SRV
	srvDesc.Buffer.NumElements = iNumAllVertices;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.StructureByteStride = iStructureByteStride;
	auto a = m_pCombinedPrototypeMeshVertices->GetDesc().Width;
	pDevice->CreateShaderResourceView(m_pCombinedPrototypeMeshVertices, &srvDesc, cpuHandle); // Vertex Srv
	pDxrResource->Apply_DescriptorHandleOffset();
#pragma endregion Create Serial SRV of VB


	Safe_Delete_Array(allVertices);
}
#endif