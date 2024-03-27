#pragma once
#include "Base.h"

NAMESPACE_(Engine)
class CSceneNode_AABB;
class CBVH : public CBase
{
protected:
	CBVH() = default;
	~CBVH() override = default;

public:
	static CBVH* Create();
	HRESULT Free() override;
public:
	void Set_Root(CSceneNode_AABB* pRoot) { m_pRoot = pRoot; }

private:
	CSceneNode_AABB* m_pRoot = nullptr;

};

_NAMESPACE