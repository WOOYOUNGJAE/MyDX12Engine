#pragma once
#include "Base.h"

NAMESPACE_(Engine)
using namespace std;
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
	void Register_SceneNode(CSceneNode* pNodeInstance, CGameObject* pContainingObj, UINT eTreeType);
	HRESULT Free() override;

private:
	// GameObj -> Array[LeafNode0, LeafNode1, ...]
	map<CGameObject*, array<CSceneNode*, SDS_TREE_TYPE_END>> m_mapGameObj_To_LeafNodes;
};

_NAMESPACE