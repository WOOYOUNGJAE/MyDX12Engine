#include "SDSManager.h"
#include "BVH.h"
#include "SceneNode_AABB.h"

IMPLEMENT_SINGLETON(CSDSManager)

void CSDSManager::Register_SceneNode(CSceneNode* pNodeInstance, CGameObject* pContainingObj, UINT eTreeType)
{
	auto mapPair = m_mapGameObj_To_LeafNodes.find(pContainingObj);

	// 리프노드 배열을 새로 만들어야할 때
	if (mapPair == m_mapGameObj_To_LeafNodes.end())
	{
		pair element = { pContainingObj, array<CSceneNode*, SDS_TREE_TYPE_END>() };
		element.second[eTreeType] = pNodeInstance;
		m_mapGameObj_To_LeafNodes.emplace(element);
	}
	else // 이미 리프노드 배열이 존재할 때
	{
		mapPair->second[eTreeType] = pNodeInstance;
	}
}

CSceneNode* CSDSManager::FindandGet_LeafNode(CGameObject* pKeyObj, UINT eTreeType)
{
	auto mapPair = m_mapGameObj_To_LeafNodes.find(pKeyObj);

	// 처음 등록되는 게임오브젝트 키 (없을 때)
	if (mapPair == m_mapGameObj_To_LeafNodes.end())
	{
		return nullptr;
	}
	else // 이미 리프노드 배열이 존재할 때
	{
		return mapPair->second[eTreeType];
	}
}

HRESULT CSDSManager::Free()
{
	HRESULT hr = S_OK;

	for (auto& mapPair : m_mapGameObj_To_LeafNodes)
	{
		for (UINT i = 0; i < SDS_TREE_TYPE_END; ++i)
		{
			Safe_Release(mapPair.second[i]);
		}
	}

	for (auto& iterBVH : m_vecAccelerationTree)
	{
		Safe_Release(iterBVH);
	}

	return hr;
}
