#pragma once

#include "Base.h"
NAMESPACE_(Engine)
class CGameObject;
class CSceneNode_AABB;
// SceneGraph�� ��尡 �� ����, BVH�� ��尡 �� ��������
class CSceneNode : public CBase
{
	NO_COPY(CSceneNode)
protected:
	CSceneNode() = default;
	~CSceneNode() override = default;

public:
	// If Binary Tree, Left Child Argument First
	static CSceneNode* Create(CSceneNode** pChildNodeArr, UINT* iNumberingArr = nullptr, UINT iArrSize = 2, bool bIsTLAS = false, CGameObject* pContainingObj = nullptr);
	HRESULT Initialize(CSceneNode** pChildNodeArr, UINT* iNumberingArr = nullptr, UINT iChildArrSize = 2, bool bIsTLAS = false);
	HRESULT Free() override;
public:
	CGameObject* Get_ContainingObj() { return m_pContainingObj; }
	virtual void Set_ContainingObj(CGameObject* pGameObj);


protected: // If Binary Tree
	CSceneNode* m_pLeftChild = nullptr;
	CSceneNode* m_pRightChild = nullptr;
protected: // Else If NOT Binary Tree
	std::vector<CSceneNode*> m_vecChildNode;
protected: // Else If Leaf
	CGameObject* m_pContainingObj = nullptr; // leaf�� �ƴϸ� nullptr

#if DXR_ON
public:
	const DXR::TLAS& Get_TLAS() { return m_TLAS; }
	void ReBuild_TLAS();
protected:
	UINT m_iNumBLAS = 0;
	UINT* m_iNumberingArr = nullptr;
	DXR::TLAS m_TLAS{};
	ID3D12Resource* m_pUploadBuffer_CombinedIndices = nullptr;
	ID3D12Resource* m_pUploadBuffer_CombinedVertices = nullptr;
	ID3D12Resource* m_pCombinedIndices = nullptr;
	ID3D12Resource* m_pCombinedVertices = nullptr;
#endif
};

_NAMESPACE