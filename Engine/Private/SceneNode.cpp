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

	if (iChildArrSize == 2)
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
	DXR::BLAS** pBlasArr = new DXR::BLAS*[iChildArrSize];
	for (UINT i = 0; i < iChildArrSize; ++i)
	{
		pBlasArr[i] = dynamic_cast<CSceneNode_AABB*>(pChildNodeArr[0])->Get_BLAS();
	}
	
	DXR_Util::Create_IB_VB_SRV_Serialized(pDevice, pBlasArr, iChildArrSize, &m_TLAS.IB_VB_SRV_startOffsetInDescriptors);
	DXR_Util::Build_TLAS0(pDevice, pCommandList, &m_TLAS.uav_TLAS, &m_TLAS.pInstanceDesc, pBlasArr, iNumberingArr, iChildArrSize);

	Safe_Delete_Array(pBlasArr);

#endif

	return hr;
}

HRESULT CSceneNode::Free()
{
#if DXR_ON
	Safe_Release(m_TLAS.uav_TLAS);
	Safe_Release(m_TLAS.pInstanceDesc);
#endif
	return S_OK;
}

void CSceneNode::Set_ContainingObj(CGameObject* pGameObj)
{
	m_pContainingObj = pGameObj;
}
