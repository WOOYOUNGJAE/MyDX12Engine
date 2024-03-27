#pragma once

#include "Base.h"
NAMESPACE_(Engine)
class CGameObject;
class CSceneNode_AABB;
// SceneGraph의 노드가 될 수도, BVH의 노드가 될 수도있음
class CSceneNode : public CBase
{
	NO_COPY(CSceneNode)
protected:
	CSceneNode() = default;
	~CSceneNode() override = default;

public:
	// If Binary Tree, Left Child Argument First
	static CSceneNode* Create(CSceneNode_AABB** pChildNodeArr, UINT iArrSize = 2, bool bIsTLAS = false);
	HRESULT Initialize(CSceneNode_AABB** pChildNodeArr, UINT iChildArrSize = 2, bool bIsTLAS = false);
	HRESULT Free() override;

protected: // If Binary Tree
	CSceneNode* m_pLeftChild = nullptr;
	CSceneNode* m_pRightChild = nullptr;
protected: // If NOT Binary Tree
	std::vector<CSceneNode*> m_vecChildNode;
protected: // If Leaf
	CGameObject* m_pContainingObj = nullptr; // leaf가 아니면 nullptr

#if DXR_ON
	DXR::TLAS* m_TLAS;
#endif
};

_NAMESPACE