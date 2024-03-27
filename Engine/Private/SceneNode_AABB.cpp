#include "SceneNode_AABB.h"
#include "GameObject.h"

CSceneNode_AABB* CSceneNode_AABB::Create(CSceneNode** pChildNodeArr, UINT iArrSize, bool bIsTLAS, CGameObject* pContainingObj)
{
	CSceneNode_AABB* pInstance = new CSceneNode_AABB;

	pInstance->Set_ContainingObj(pContainingObj);

	if (FAILED(pInstance->Initialize(pChildNodeArr, iArrSize, bIsTLAS)))
	{
		Safe_Release(pInstance);
		MSG_BOX("SceneNode : Failed to Init");
		return pInstance;
	}

	return pInstance;
}

HRESULT CSceneNode_AABB::Initialize(CSceneNode** pChildNodeArr, UINT iChildArrSize, bool bIsTLAS)
{
	HRESULT hr = S_OK;

	hr = CSceneNode::Initialize(pChildNodeArr, iChildArrSize, bIsTLAS);
	if (FAILED(hr)) { return hr; }


	// TODO: Make AABB


	return hr;
}

HRESULT CSceneNode_AABB::Free()
{
	return CSceneNode::Free();
}

void CSceneNode_AABB::Set_ContainingObj(CGameObject* pGameObj)
{
	if (pGameObj == nullptr)
	{
		return;
	}

	CSceneNode::Set_ContainingObj(pGameObj);

#if DXR_ON
	m_pContainingBLAS = m_pContainingObj->Get_BLAS_Ptr();
#endif

}
