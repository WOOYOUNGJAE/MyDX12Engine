#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
class CBVH;
class CGameObject;
class CSceneNode;
// Spatial Data Structure Manager
class CSDSManager : public CBase
{
	DECLARE_SINGLETON(CSDSManager)

protected:
	CSDSManager() = default;
	~CSDSManager() override = default;

public:
	HRESULT Free() override;
public:
	void Register_SceneNode(CSceneNode* pNodeInstance, CGameObject* pContainingObj, UINT eTreeType);
	CSceneNode* FindandGet_LeafNode(CGameObject* pKeyObj, UINT eTreeType);

private:
	// GameObj -> Array[LeafNode0, LeafNode1, ...]
	map<CGameObject*, array<CSceneNode*, SDS_TREE_TYPE_END>> m_mapGameObj_To_LeafNodes;

#if DXR_ON
public:
	void Push_AccelerationTree(CBVH* pASTreeInstance) { m_vecAccelerationTree.emplace_back(pASTreeInstance); }
	const vector<CBVH*>& Get_vecAccelerationTree() { return m_vecAccelerationTree; }
private:
	vector<CBVH*> m_vecAccelerationTree; // Acceleration Structure Tree
#endif
};

_NAMESPACE