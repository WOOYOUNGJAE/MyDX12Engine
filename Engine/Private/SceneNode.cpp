#include "SceneNode.h"
#include "GameObject.h"
#include "SceneNode_AABB.h"
#include "DeviceResource.h"
#include "Device_Utils.h"
#include "DXRRenderer.h"

CSceneNode* CSceneNode::Create(CSceneNode** pChildNodeArr, UINT* iNumberingArr, UINT iArrSize, bool bIsTLAS, CGameObject* pContainingObj)
{
	CSceneNode* pInstance = new CSceneNode;

	pInstance->Set_ContainingObj(pContainingObj);

	if (FAILED(pInstance->Initialize(pChildNodeArr, iNumberingArr, iArrSize)))
	{
		Safe_Release(pInstance);
		MSG_BOX("SceneNode : Failed to Init");
		return pInstance;
	}

	return pInstance;
}

//static ID3D12Resource* m_bottomLevelAccelerationStructure = nullptr;
HRESULT CSceneNode::Initialize(CSceneNode** pChildNodeArr, UINT* iNumberingArr, UINT iChildArrSize, bool bIsTLAS)
{
	HRESULT hr = S_OK;

#if DXR_ON
	if (iChildArrSize == 2 && bIsTLAS == false) // Acceleration Structure가 아닌 일반 bvh의 노드
#else
	if (iChildArrSize == 2)
#endif
	{
		m_pLeftChild = pChildNodeArr[0];
		m_pRightChild = pChildNodeArr[1];
	}
	else // 바이너리 트리가 아닌 경우
	{
		for (UINT i = 0; i < iChildArrSize; ++i)
		{
			m_vecChildNode.emplace_back(pChildNodeArr[i]);
		}
	}

#if DXR_ON
	// DXR을 위한 노드가 아니면 Init 마무리
	if (bIsTLAS == false)
	{
		return hr;
	}
	CDXRResource* pDXRResource = CDXRResource::Get_Instance();

	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
    ID3D12GraphicsCommandList4* pCommandList = pDXRResource->Get_CommandList4();
	m_iNumBLAS = iChildArrSize;
	m_iNumberingArr = new UINT[iChildArrSize];
	memcpy(m_iNumberingArr, iNumberingArr, sizeof(UINT) * iChildArrSize);
	DXR::BLAS_INFOS** pBlasArr = new DXR::BLAS_INFOS*[iChildArrSize];
	CGameObject** pGameObjArr = new CGameObject*[iChildArrSize];
	UINT iNumAllIndices = 0;
	UINT iNumAllVertices = 0;
	for (UINT i = 0; i < iChildArrSize; ++i)
	{
		pBlasArr[i] = dynamic_cast<CSceneNode_AABB*>(m_vecChildNode[i])->Get_BLAS();
		pGameObjArr[i] = m_vecChildNode[i]->Get_ContainingObj();

		iNumAllIndices += pBlasArr[i]->vecIndices.size();
		iNumAllVertices += pBlasArr[i]->vecVertices.size();
	}


	//std::vector<UINT16> vecAllIndices;
	//auto iterIndicesDest = vecAllIndices.begin();
	//UINT iStartIndex_in_SRV = 0;
	//for (UINT i = 0; i < iChildArrSize; ++i)
	//{
	//	const auto& vecSrc = pBlasArr[i]->vecIndices;
	//	pChildNodeArr[i]->Get_ContainingObj()->Get_BLAS_Ptr()->iStartIndex_in_IB_SRV = iStartIndex_in_SRV;
	//	
	//	for (UINT j = 0; j < pBlasArr[i]->vecIndices.size(); ++j)
	//	{
	//		vecAllIndices.emplace_back(vecSrc[j]);
	//	}
	//	INT iLackCount = 4 - (pBlasArr[i]->vecIndices.size() % 4);
	//	if (iLackCount == 4) // 딱 4바이트 단위로 맞아 떨어지면
	//	{
	//		iStartIndex_in_SRV = vecAllIndices.size();
	//		continue;
	//	}
	//	for (UINT i = 0; i < iLackCount; ++i)
	//	{
	//		vecAllIndices.emplace_back(0); // 3의 배수에 2개가 부족하면
	//	}
	//	iStartIndex_in_SRV = vecAllIndices.size();
	//}
	//vecAllIndices.shrink_to_fit();
	//MyUtils::Create_Buffer_Default(pDevice, pCommandList, vecAllIndices.data(), sizeof(UINT16) * vecAllIndices.size(), &m_pUploadBuffer_CombinedIndices, &m_pCombinedIndices);


	//iStartIndex_in_SRV = 0;
	//VertexPositionNormalColorTexture* allVertices = new VertexPositionNormalColorTexture[iNumAllVertices];
	//VertexPositionNormalColorTexture* pVerticesCopyDst = allVertices;
	//for (UINT i = 0; i < iChildArrSize; ++i)
	//{
	//	pChildNodeArr[i]->Get_ContainingObj()->Get_BLAS_Ptr()->iStartIndex_in_VB_SRV = iStartIndex_in_SRV;
	//	std::copy(pBlasArr[i]->vecVertices.begin(), pBlasArr[i]->vecVertices.end(), pVerticesCopyDst);
	//	UINT iCurVerticesSize = pBlasArr[i]->vecVertices.size();
	//	pVerticesCopyDst += iCurVerticesSize;
	//	iStartIndex_in_SRV += iCurVerticesSize;
	//}
	//MyUtils::Create_Buffer_Default(pDevice, pCommandList, allVertices, sizeof(VertexPositionNormalColorTexture) * iNumAllVertices, &m_pUploadBuffer_CombinedVertices, &m_pCombinedVertices);

	//CDXRResource* pDxrResource = CDXRResource::Get_Instance();
	//pDxrResource->Close_CommandList();
	//pDxrResource->Execute_CommandList();
	//pDxrResource->Flush_CommandQueue();
	//pDxrResource->Reset_CommandList();

	//DXR_Util::Create_IB_VB_SRV_Serialized(pDevice, vecAllIndices.size(), iNumAllVertices, m_pCombinedIndices, m_pCombinedVertices, pBlasArr[0]->dxrGeometryDesc.Triangles.VertexBuffer.StrideInBytes, );
	DXR_Util::Build_TLAS0(pDevice, pCommandList, &m_TLAS.uav_TLAS, &m_TLAS.pInstanceDesc, pGameObjArr, iNumberingArr, iChildArrSize);

	for (UINT i = 0; i < m_iNumBLAS; ++i)
	{
		pBlasArr[i]->vecVertices.clear();
		pBlasArr[i]->vecIndices.clear();
	}

	//Safe_Delete_Array(allVertices);
	Safe_Delete_Array(pBlasArr);
	Safe_Delete_Array(pGameObjArr);


#endif

	return hr;
}

HRESULT CSceneNode::Free()
{
#if DXR_ON
	Safe_Release(m_pUploadBuffer_CombinedIndices);
	Safe_Release(m_pUploadBuffer_CombinedVertices);
	Safe_Release(m_pCombinedIndices);
	Safe_Release(m_pCombinedVertices);
	Safe_Delete_Array(m_iNumberingArr);
	Safe_Release(m_TLAS.uav_TLAS);
	Safe_Release(m_TLAS.pInstanceDesc);
#endif
	return S_OK;
}

void CSceneNode::Set_ContainingObj(CGameObject* pGameObj)
{
	m_pContainingObj = pGameObj;
}

#if DXR_ON
void CSceneNode::ReBuild_TLAS()
{
	CDXRResource* pDXRResource = CDXRResource::Get_Instance();
	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
	ID3D12GraphicsCommandList4* pCommandList = pDXRResource->Get_CommandList4();

	CGameObject** pGameObjArr = new CGameObject * [m_iNumBLAS];
	for (UINT i = 0; i < m_iNumBLAS; ++i)
	{
		pGameObjArr[i] = m_vecChildNode[i]->Get_ContainingObj();
	}

	Safe_Release(m_TLAS.pInstanceDesc);
	Safe_Release(m_TLAS.uav_TLAS);
	DXR_Util::Build_TLAS0(pDevice, pCommandList, &m_TLAS.uav_TLAS, &m_TLAS.pInstanceDesc, pGameObjArr, m_iNumberingArr, m_iNumBLAS);

	Safe_Delete_Array(pGameObjArr);
}
#endif