#include "SceneNode.h"
#include "GameObject.h"
#include "SceneNode_AABB.h"
#include "DeviceResource.h"
#include "Device_Utils.h"

CSceneNode* CSceneNode::Create(CSceneNode** pChildNodeArr, UINT iArrSize, bool bIsTLAS, CGameObject* pContainingObj)
{
	CSceneNode* pInstance = new CSceneNode;

	pInstance->m_pContainingObj = pContainingObj;

	if (FAILED(pInstance->Initialize(pChildNodeArr, iArrSize)))
	{
		Safe_Release(pInstance);
		MSG_BOX("SceneNode : Failed to Init");
		return pInstance;
	}

	return pInstance;
}

HRESULT CSceneNode::Initialize(CSceneNode** pChildNodeArr, UINT iChildArrSize, bool bIsTLAS)
{
	HRESULT hr = S_OK;

	if (iChildArrSize == 2)
	{
		m_pLeftChild = pChildNodeArr[0];
		m_pRightChild = pChildNodeArr[1];
	}
	else if (iChildArrSize > 2) // �ټ��� child
	{
		for (UINT i = 0; i < iChildArrSize; ++i)
		{
			m_vecChildNode.emplace_back(pChildNodeArr[i]);
		}
	}

#if DXR_ON
	// DXR�� ���� ��尡 �ƴϸ� Init ������
	if (bIsTLAS == false)
	{
		return hr;
	}

	ID3D12Device5* pDevice = CDeviceResource::Get_Instance()->Get_Device5();
	vector<ID3D12Resource*> vecUAV_BLAS;
	vecUAV_BLAS.resize(iChildArrSize);
	for (UINT i = 0; i < iChildArrSize; ++i)
	{
		vecUAV_BLAS.emplace_back(dynamic_cast<CSceneNode_AABB*>(pChildNodeArr[i])->Get_BLAS()->uav_BLAS);
	}
	::Engine::DXR_Util::Build_TLAS(pDevice, &m_TLAS->uav_TLAS, vecUAV_BLAS.data(), iChildArrSize);
#endif

	return hr;
}

HRESULT CSceneNode::Free()
{
	return S_OK;
}

void CSceneNode::Set_ContainingObj(CGameObject* pGameObj)
{
	m_pContainingObj = pGameObj;
}
