#include "SDSManager.h"
#include "BVH.h"
#include "SceneNode_AABB.h"

IMPLEMENT_SINGLETON(CSDSManager)

void CSDSManager::Register_SceneNode(CSceneNode* pNodeInstance, CGameObject* pContainingObj, UINT eTreeType)
{
	auto mapPair = m_mapGameObj_To_LeafNodes.find(pContainingObj);

	// ������� �迭�� ���� �������� ��
	if (mapPair == m_mapGameObj_To_LeafNodes.end())
	{
		pair element = { pContainingObj, array<CSceneNode*, SDS_TREE_TYPE_END>() };
		element.second[eTreeType] = pNodeInstance;
		m_mapGameObj_To_LeafNodes.emplace(element);
	}
	else // �̹� ������� �迭�� ������ ��
	{
		mapPair->second[eTreeType] = pNodeInstance;
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
	return hr;
}
