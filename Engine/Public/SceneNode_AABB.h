#pragma once

#include "SceneNode.h"
NAMESPACE_(Engine)
class CGameObject;
// 주로 BVH 용도를 위한 AABB 노드
// DXR에서는 BLAS을 위해서만 활용
class CSceneNode_AABB : public CSceneNode
{
protected:
	CSceneNode_AABB() = default;
	~CSceneNode_AABB() override = default;

public:
	// If Binary Tree, Left Child Argument First
	static CSceneNode_AABB* Create(CSceneNode** pChildNodeArr, UINT
	                               * iNumberingArr = nullptr, UINT iArrSize = 2, bool bIsTLAS = false, CGameObject* pContainingObj = nullptr);
	HRESULT Initialize(CSceneNode** pChildNodeArr, UINT* iNumberingArr = nullptr, UINT iChildArrSize = 2, bool bIsTLAS = false);
	HRESULT Free() override;
public:
	void Set_ContainingObj(CGameObject* pGameObj) override;

private:
	Vector3 mBoxMin = Vector3::Zero;
	Vector3 mBoxMax = Vector3::Zero;


#if DXR_ON
public:
	DXR::BLAS*& Get_BLAS() { return m_pContainingBLAS; }
private:
	DXR::BLAS* m_pContainingBLAS = nullptr; // pointer
#endif
};

_NAMESPACE